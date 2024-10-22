#include <linux/if_ether.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h> 
#include <netinet/ip_icmp.h> 
#include <net/ethernet.h> 
#include "config.h"
#include "conv.h"

void process_udp_packet(unsigned char* buffer, int size){
	struct udphdr *udph = (struct udphdr *)(buffer + sizeof(struct ethhdr) + sizeof(struct iphdr));
	printf("\tUDP Header\n");
	printf("\t\tSource Port: %u\n", ntohs(udph->uh_sport));
	printf("\t\tDestination Port: %u\n", ntohs(udph->uh_dport));
	printf("\t\tDatagram size: %u\n", ntohs(udph->uh_ulen));
	printf("\t\tChecksum: %u\n", ntohs(udph->uh_sum));

	unsigned long payload_size = size - sizeof(struct ethhdr) - sizeof(struct iphdr) - sizeof(struct udphdr);
	printf("\t\tPayload size: %lu\n", payload_size);
	
	if (SHOW_PAYLOAD && payload_size){
		unsigned char* payload = buffer +  sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr);
		payload[payload_size - 1] = '\0'; // to avoid invalid memory access 
		printf("\t\tPayload data: ");
		try_print(payload, payload_size);
	}

}


