
#include "conv.h"
#include "string.h"
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <stdio.h>

char* printable = "!@#$%^&*()+= `<>[]{}/\\-_'\":";
void try_print(unsigned char* buffer, int size){
	for(int i = 0; i<size; i++){
		char c = '?';
		if (buffer[i] >= 'a' && buffer[i] <= 'z'){
			c = buffer[i];
		} else if (buffer[i] >= 'A' && buffer[i] <= 'Z'){
			c = buffer[i];
		} else if (buffer[i] >= '0' && buffer[i] <= '9'){
			c = buffer[i];
		} else if (strchr(printable, buffer[i])){
			c = buffer[i];
		}

		printf("%c", c);
	}
	printf("\n");
}
void print_mac_address(unsigned char *mac) {
    printf("%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

char *ethernet_protocol_conv(unsigned short protocol) {
    switch (ntohs(protocol)) {
        case ETH_P_IP:
            return("IPv4");
            break;
        case ETH_P_ARP:
            return("ARP");
            break;
        case ETH_P_RARP:
            return("RARP");
            break;
        case ETH_P_IPV6:
            return("IPv6");
            break;
        default:
            return("Unknown");
            break;
    }
}
char *ip_protocol_conv(unsigned short protocol) {
    switch (ntohs(protocol)) {
        case IPPROTO_ICMP:
            return("ICMP");
            break;
        case IPPROTO_TCP:
            return("TCP");
            break;
        case IPPROTO_UDP:
            return("UDP");
            break;
        default:
            return("Unknown");
            break;
    }
}
