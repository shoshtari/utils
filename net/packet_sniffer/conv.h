




#ifndef CONV_INC 
#define CONV_INC 
void print_mac_address(unsigned char *mac) ;
char* ethernet_protocol_conv(unsigned short protocol);
char* ip_protocol_conv(unsigned short protocol);
void try_print(unsigned char* buffer, int size);
#endif // ! 
