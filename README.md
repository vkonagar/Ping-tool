Pinger-lite
===========

#### This application requires Root permissions to run (Creating a Raw socket needs the user to be a sudoer)

* This application is developed using raw sockets in C 
* An ICMP packet is crafted (Echo request), its then encapsulated inside an IP datagram, and sent.
* The destination ICMP stack will respond to the ICMP packet by sending back the reply ( Echo Reply ). Application waits for the echo reply and confirms the availabilty of the remote host. 
* Round Trip Time is calculated by calculating the difference between the timestamp when the request packet is sent and the timestamp when the reply is received. 
* I have used IO Multiplexing ( select system call ) for waiting until a timeout has been reached. 

## Usage: 
* You can use this Application to substitute your default ping application.
* Download this repo as Zip and Extract it.
* Compile ping.c file
* Run the output file using sudo
* Make sure you enter the IP address, not the Domain Name
