#include "md5.h"

char* calculate_md5(char* data, int n) {
    // Allocate memory for the MD5 hash output (32 hex characters + null terminator)
    char* md5_string = malloc(33);
    if (md5_string == NULL) {
        perror("Unable to allocate memory for MD5 string");
        return NULL;
    }

    unsigned char md5_hash[MD5_DIGEST_LENGTH];  // MD5 produces a 16-byte hash

    // Calculate the MD5 hash
    MD5((unsigned char*)data, n, md5_hash);

    // Convert the hash to a hexadecimal string
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        snprintf(&md5_string[i * 2], 3, "%02x", md5_hash[i]);
    }

    return md5_string;  // Return the MD5 string
}
