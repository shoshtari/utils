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
#include <net/ethernet.h> 
#include "config.h"
#include "conv.h"

void process_tcp_packet(unsigned char* buffer, int size){
	struct tcphdr *tcph = (struct tcphdr *)(buffer + sizeof(struct ethhdr) + sizeof(struct iphdr));
	printf("\tTCP Header\n");
	printf("\t\tSource Port: %u\n", ntohs(tcph->th_sport));
	printf("\t\tDestination Port: %u\n", ntohs(tcph->th_dport));
	printf("\t\tSequence Number: %u\n", ntohl(tcph->th_seq));
	printf("\t\tAck Number: %u\n", ntohl(tcph->th_ack));
	printf("\t\tFlags:\n");
	printf("\t\t\tFIN:%d\n", tcph->fin);
	printf("\t\t\tSYN:%d\n", tcph->syn);
	printf("\t\t\tRST:%d\n", tcph->rst);
	printf("\t\t\tPSH:%d\n", tcph->psh);
	printf("\t\t\tACK:%d\n", tcph->ack);
	printf("\t\t\tURG:%d\n", tcph->urg);
	printf("\t\tWindow Size:%u\n", ntohs(tcph->th_win));
	printf("\t\tChecksum:%u\n", ntohs(tcph->th_sum));
	unsigned long payload_size = size - sizeof(struct ethhdr) - sizeof(struct iphdr) - sizeof(struct tcphdr);
	printf("\t\tPayload size: %lu\n", payload_size);
	
	if (SHOW_PAYLOAD && payload_size){
		unsigned char* payload = buffer +  sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr);
		payload[payload_size - 1] = '\0'; // to avoid invalid memory access 
		printf("\t\tPayload data: ");
		try_print(payload, payload_size);
	}

}


