#include <linux/if_ether.h>
#include <netinet/in.h>
#include <stdint.h>
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

void process_icmp_packet(unsigned char* buffer, int size){
	struct icmphdr *icmph = (struct icmphdr *)(buffer + sizeof(struct ethhdr) + sizeof(struct iphdr));
	printf("\tICMP Header\n");
	printf("\t\tType: %u\n", icmph->type);
	printf("\t\tCode: %u\n", icmph->code);
	printf("\t\tID: %u\n", ntohs(icmph->un.echo.id));
	printf("\t\tSequence: %u\n", ntohs(icmph->un.echo.sequence));
	printf("\t\tChecksum: %u\n", ntohs(icmph->checksum));
	unsigned long payload_size = size - sizeof(struct ethhdr) - sizeof(struct iphdr) - sizeof(struct icmphdr);
	printf("\t\tPayload size: %lu\n", payload_size);
	
	if (SHOW_PAYLOAD && payload_size){
		unsigned char* payload = buffer +  sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct icmphdr);
		payload[payload_size - 1] = '\0'; // to avoid invalid memory access 
		printf("\t\tPayload data: ");
		try_print(payload, payload_size);
	}

}


