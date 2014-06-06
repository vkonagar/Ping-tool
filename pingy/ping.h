#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/ip.h>
#include<netinet/ip_icmp.h>

#define DATA_LENGTH 16 // this is not portable
#define MAX_IP_SIZE 20
#define ECHO_REQUEST 8
#define TOTAL_ICMP_SIZE 24
#define PACKET_SIZE 44
#define ICMP_ECHO_REPLY 0
#define ICMP_DEST_HOST_UNREACH 3

struct icmp_hdr
{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t id;
	uint16_t seq;
	uint8_t data[DATA_LENGTH];
};
