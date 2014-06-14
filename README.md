Pinger-lite
===========

#### This application needs Root permissions to run (Creating a Raw socket needs a user to be a sudoer)

This application is developed using raw socket which provides an access for creating packets which would protocols other than TCP,UDP. I formed an ICMP packet (Echo request) and then encapsulated it inside an IP datagram and sent to the destination. The destination ICMP stack will respond to the ICMP packet by sending back the reply ( Echo Reply ). Application waits for the echo reply and confirms the availabilty of the remote host. Round Trip Time is calculated by sending the current time in the request packet and calculating the time when the reply is received and taking the difference of them. I have used a selector ( select system call ) for waiting untill a timeout has been reached. 
