#include "md5.h"

// Function to convert MD5 hash to a hexadecimal string
char* calculate_md5(unsigned char* data, int n) {
	unsigned char md[MD5_DIGEST_LENGTH];
    MD5((unsigned char*)data, n, md);

	char* ans = (char*) malloc(50);
	char buf[10];

	sprintf(buf, "%02x", md[0]);
	strcpy(ans, buf);
    for(int i = 1; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(buf, "%02x", md[i]);
		strcat(ans, buf);
    }
	return ans;
}

