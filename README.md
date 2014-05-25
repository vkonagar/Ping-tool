Pinger-lite
===========

Pinger is a Unix Application which can be used to ping remote hosts with their IP addresses to check their availability.
* ICMP Echo requests will be sent for every second when the ping is initiated
* ICMP Echo replies are handled in the application and RTT is calculated.
* Sending timestamp will be added to the data portion of the ICMP packet and sent, inorder to calculate the rtt when the packet is received.
* Raw Socket is used with the protocol IPPROTO_ICMP, so that the ICMP protocol number will be added to the IP header's protocol field.
* ICMP packet is constructed manually and sent through the socket.
