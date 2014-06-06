#include "ping.h"

int no_of_sent_packets = 0 ;
int no_of_replies_rcvd = 0 ;


uint16_t checksum( uint8_t* data, int length) 
{
	uint32_t sum = 0;
	
	uint16_t* pointer = (uint16_t*)data;
	uint16_t answer = 0;

	int temp = length;
	while( temp > 1 )
	{
		sum += *pointer++;
		temp -= 2;
	}
	
	// If the no of bytes are odd, then mop up the byte and add to the sum
	if( temp == 1 )
	{	
		*((uint8_t*)(&answer)) = *((uint8_t*)pointer);
		sum += answer;
	}

	// Now accumulated carries are shifted and added
	// Shift higher 16 bits to lower 16 bits in the sum and add with the existing sum

	sum = (sum >> 16) + (sum & 0x0000FFFF);	// Add the higher order bits to lower order bits

	sum = (sum >> 16) + (sum & 0x0000FFFF); // Add the carry which could be generated due to the above step
	
	answer = (uint16_t)~sum;	
	
	return answer;
}

void signal_handler()
{
	printf("No of Sent Packets : %d\n",no_of_sent_packets);	
	printf("No of Recvd Packets : %d\n",no_of_replies_rcvd);
	printf("Dropped packets : %d\n",no_of_sent_packets-no_of_replies_rcvd);
	exit(1);
}

void store_time(uint8_t* src, uint8_t* dst)
{
	int i;
	// 16 bytes are copied from the timeval structure to the data of the ICMP packet
	for(i=0;i<16;i++)
	{
		*dst++ = *src++;
	}
}

struct timeval get_difference(struct timeval t1, struct timeval t2)
{
	long int diff = ( t2.tv_usec + (1000000*t2.tv_sec) ) - ( t1.tv_usec + (1000000*t1.tv_sec) );
	struct timeval result;
	result.tv_sec = diff/1000000;
	result.tv_usec = diff%1000000;
	return result;
}

// Check's wheather the packet has any errors. If any, appropriate error code is returned
int check_packet(uint8_t* buffer, int seq_no)
{
	
	struct icmp_hdr* i_hdr = (struct icmp_hdr*)(buffer+sizeof(struct iphdr));
	// Check if the ICMP packet is a Destination unreacable message

	if( i_hdr->type == 3 )
	{
		printf("Dest host %d and %d \n",i_hdr->type,i_hdr->code);
		//Dest host unreach, So now check the payload and match
		return 1;
	}

	// Check if the packet belongs to the same process and has the seq no of the last packet sent
	int pid = htons((uint16_t)getpid()); 
	if( i_hdr->seq != htons(seq_no) || i_hdr->id != pid )
	{
	//	printf("Seq number Problem\n");
		return 2;
	}

	// Check if the ICMP packet is echo reply
	if( i_hdr->type != ICMP_ECHO_REPLY )
	{
		printf("Icmp not reply %d and %d \n",i_hdr->type,i_hdr->code);
		return 3;
	}

	uint16_t return_cksum = i_hdr->checksum;
	i_hdr->checksum = 0;

	uint16_t cal_cksum = checksum( buffer+ sizeof(struct iphdr), TOTAL_ICMP_SIZE );
	if( return_cksum != cal_cksum )
	{
		// Discard the packet, Incorrect checksum
		printf("Checksum Error\n");
		return 4;
	}
	return -1;
}

void send_packet(int sock, struct sockaddr_in remote_host,int length,int seq_no)
{
		struct timeval time_start;
	
		uint8_t buffer[TOTAL_ICMP_SIZE];
		uint16_t pid = htons((uint16_t)getpid());
		struct icmp_hdr* icmp_p = (struct icmp_hdr*)buffer;	
		icmp_p->type = ECHO_REQUEST;
		icmp_p->code = 0;
		icmp_p->checksum = htons(0);
		icmp_p->id = pid;
		icmp_p->seq = htons((uint16_t)seq_no);
		memset(icmp_p->data,0,DATA_LENGTH);	
		
		// Get the start time
		gettimeofday(&time_start,NULL);

		// Store it in the packet
		uint8_t* t_strt_b = (uint8_t*)&time_start;

		// Src(1st arg) --> dst(2nd arg)
		store_time(t_strt_b,icmp_p->data);

		icmp_p->checksum = checksum(buffer, TOTAL_ICMP_SIZE);
	
		sendto(sock, buffer, TOTAL_ICMP_SIZE, 0, (struct sockaddr*)&remote_host, length);
		// Wait for 1 sec and send the packet
		no_of_sent_packets++;
}
int main()
{
	int i;
	int seq_no;
	struct timeval time_start,time_end;
	// Create a socket which listens only for the ICMP packets
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if( sock < 0 )
	{
		printf("Error in creating the socket \n");
		exit(0);
	}
	struct sockaddr_in remote_host;
	int length = sizeof(remote_host);
	remote_host.sin_family = AF_INET;
	
	signal(SIGINT,signal_handler);

	uint8_t ip[MAX_IP_SIZE];
	printf("Enter the IP Address: ");
	scanf("%s",ip);	
	// Add IP address of the remote host
	if( inet_pton(AF_INET, ip, &remote_host.sin_addr)  == 0 )
	{
		printf("Please enter the IP address in dotted decimal format!\n");
		return;
	}

	uint8_t recvBuffer[PACKET_SIZE];

	printf("Pinging %s with 44 bytes of data... \n",ip);

	int time_out_count = 0;

	// Initialize the FDSET, to listen on the socket asynchronously.
	// If any packet arrives on the socket, we perform the checking and finally accepting the packet
	// If not then we wait untill the timelimit is finished and send the packet again.
	// This is done by specifying timeval in the select option

	fd_set r_set;

	FD_ZERO(&r_set);
	
	int max_fd = sock+1;
	
	struct sockaddr_in recv_host;

	for(;;)
	{
		time_out_count = 0;
		
		send_packet(sock,remote_host,length,seq_no);
		
		while(1)
		{
		
			FD_ZERO(&r_set);
			
			FD_SET(sock,&r_set);
			
	
	// If the packet is not received within 2 seconds, the select call returns, so that we can send one more packet
	
			struct timeval timeout;
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;
		
			select(max_fd, &r_set, NULL, NULL, &timeout); 

			if( FD_ISSET(sock, &r_set) )
			{
				recvfrom(sock, recvBuffer, PACKET_SIZE , 0, (struct sockaddr*)&recv_host, &length);
			}
			else
			{
				printf("Request timed out! \n");
				break;
			}

			int valid = check_packet(recvBuffer,seq_no);
			
			if( valid == 1 )
			{
				printf("Destination Host Unreachable\n");
				break;
			}

			if( valid == 2 || valid == 3 || valid == 4 )
			{
				time_out_count++;
				if( time_out_count == 50 )
				{
					printf("Request timed out! \n");
					break;
				}
				continue;
			}
	
			gettimeofday(&time_end,NULL);

			struct icmp_hdr* hdr = (struct icmp_hdr*)(recvBuffer+sizeof(struct iphdr));
	
			// Using the same time_start variable for storing the extracted time from packet
			uint8_t* t_end_b = (uint8_t*)&time_start;
		
			store_time((uint8_t*)hdr->data,t_end_b);
		
			struct timeval result = get_difference(time_start,time_end);

			struct iphdr* ip_h = (struct iphdr*)recvBuffer;
			
			inet_ntop(AF_INET, &(recv_host.sin_addr), ip, 10 );

			printf("Reply from %s , ttl = %d, id = %d, seq = %d RTT : %ld.%ld\n", \
					 ip, ip_h->ttl, hdr->id,\
					seq_no, result.tv_sec,result.tv_usec);
			no_of_replies_rcvd++;
			break;	
		}
		seq_no++;
		sleep(1);
	}
}
