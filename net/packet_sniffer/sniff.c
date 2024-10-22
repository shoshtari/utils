#include <linux/if_ether.h>
#include <netinet/in.h>
#include "md5.h"
#include "udp.h"
#include "conv.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h> 
#include <netpacket/packet.h>
#include <netinet/ip_icmp.h> 
#include "config.h"
#include "tcp.h"
#include "icmp.h"
#include "udp.h"
#include "string.h"
#include <net/ethernet.h> 


char** processed_md5 = NULL;
int head = 0;
int max_size = 100;
int is_processed_before(char* md5){
	if (processed_md5 == NULL){
		processed_md5 = malloc(sizeof(char**) * max_size);
		for(int i = 0; i<max_size; i++){
			processed_md5[i] = NULL;
		}
	}

	for(int i = 0; i<max_size; i++){
		if (processed_md5[i] == NULL){
			continue;
		}
		if (strstr(processed_md5[i], md5)){
			return 1;
		}
	}
	if (processed_md5[head] == NULL){
		free(processed_md5[head]);
	}
	processed_md5[head] = md5;
	return 0;
}

void process_packet(unsigned char *buffer, int size) {

	struct ethhdr *ethh = (struct ethhdr *)(buffer);
    struct iphdr *iph = (struct iphdr *)(buffer + sizeof(struct ethhdr)); 

	/*if (iph->daddr != ((1 << 24) + (1 << 16) + 10)){*/
	/*	return;*/
	/*}*/

	if (is_processed_before(calculate_md5(buffer, size))){
		printf("DUP packet recieved, ignoring\n");
		return;
	} 

	printf("####################\n");
	printf("new packet sniffed\n");
	printf("\tETH Header\n");

    printf("\t\tSource : ");
	print_mac_address(ethh->h_source);
	printf("\n");

    printf("\t\tSource : ");
	print_mac_address(ethh->h_dest);
	printf("\n");

    printf("\t\tProtocol: %s\n", ethernet_protocol_conv(ethh->h_proto));

    printf("\tIP Header\n");
    printf("\t\tVersion: %d\n", iph->version);
    printf("\t\tSource IP: %s\n", inet_ntoa(*(struct in_addr *)&iph->saddr));
    printf("\t\tDestination IP: %s\n", inet_ntoa(*(struct in_addr *)&iph->daddr));
    printf("\t\tProtocol: %s\n", ip_protocol_conv(iph->protocol));


    switch (iph->protocol) {
        case IPPROTO_ICMP:  
			process_icmp_packet(buffer, size);
            break;
        case IPPROTO_TCP:  
			process_tcp_packet(buffer, size);
            break;
        case IPPROTO_UDP:
			process_udp_packet(buffer, size);
            break;
        default:
            break;
    }
}

int main() {
    int sock_raw;
    struct sockaddr saddr;
    int saddr_len = sizeof(saddr);
    unsigned char *buffer = (unsigned char *)malloc(65536); 

    sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock_raw < 0) {
        perror("can't create socket");
        return 1;
    }

	int value = 1;
	setsockopt(sock_raw, SOL_PACKET, PACKET_IGNORE_OUTGOING, &value,  sizeof(value));

	printf("Waiting for packets");
    while (1) {
        int packet_size = recvfrom(sock_raw, buffer, 65536, 0, &saddr, (socklen_t *)&saddr_len);
        if (packet_size < 0) {
            perror("can't recv packet");
            return 1;
        }
        process_packet(buffer, packet_size);
    }

    close(sock_raw);
    free(buffer);
    return 0;
}

