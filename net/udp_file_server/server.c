#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "file_utils.h"
#include "net/socket_manager.h"
#include "table.h"

#define DIR_TO_SERVE "/home/morteza/Pictures/wallpapers"

socket_manager* manager;

char* gen_file_list(dir_files files, int limit, int offset) {
    int length = 10000;
    char* ans = malloc(length);
    int occupied = 0;

    limit += offset;
    for (int i = offset; i < files.filecounts && (i < limit || limit == -1); i++) {
        char* new_file_entry = gen_file_list_entry(files, i);
        if (new_file_entry == NULL) {
            printf("file entry is null\n");
            break;
        }
        int new_char_count = strlen(new_file_entry);
        if ((new_char_count + occupied) > DG_MAXSIZE) {
            break;
        }
        while (new_char_count + occupied + 10 >= length) {
            ans = realloc_pointer(ans, length);
            length *= 2;
        }

        memcpy(ans + occupied, new_file_entry, new_char_count);
        // strcat(ans, new_file_entry);
        occupied += strlen(new_file_entry);
        free(new_file_entry);
    }
    ans[occupied] = '\0';

    return ans;
}

void serve(dir_files files) {
    int recv_len;

    printf("serving %d files\n", files.filecounts);

    fflush(stdout);

    while (1) {
        Packet* receivedPacket = app_recv(manager);

        char* buf = receivedPacket->buffer + PROTOCOL_OVERHEAD;
        char* command = strtok(buf, "-");

        char* text = "unknown command";
        if (strcmp(command, "ping") == 0) {
            text = "pong";
        } else if (strcmp(command, "list") == 0) {
            char* limitStr = strtok(NULL, "-");
            char* offsetStr = strtok(NULL, "-");
            int limit = atoi(limitStr);
            int offset = atoi(offsetStr);

            text = gen_file_list(files, limit, offset);
        } else if (strcmp(command, "get") == 0) {
            int fileid = atoi(strtok(NULL, "-")), start = atoi(strtok(NULL, "-")), size = atoi(strtok(NULL, "-"));
            size = files.files[fileid].size < size ? files.files[fileid].size : size;
            printf("%d %d %d\n", fileid, start, size);

            app_send(manager, newPacket(receivedPacket->addr, files.files[fileid].data + start, size));
            continue;
        } else if (strcmp(command, "exit") == 0) {
            return;
        }

        app_send(manager, newPacket(receivedPacket->addr, text, strlen(text)));
        // free(text);
    }
}

int main(int argc, char** argv) {
    printf("running server...\n");

    if (argc != 2) {
        perror("you must run this like ./server PORT");
        exit(EXIT_FAILURE);
    }

    unsigned int port = atoi(argv[1]);
    int sockfd;
    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_addr;

    // Zero out the structure
    memset((char*)&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    server_addr.sin_port = htons(port);

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int)) <
        0) {
        perror("can't set reuse address");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the port
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) ==
        -1) {
        perror("couldn't bind socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    dir_files files;

    read_files(DIR_TO_SERVE, &files);
    print_files(files);

    manager = new_socket_manager(sockfd);
    pthread_t pid;
    if (pthread_create(&pid, NULL, run_recv_daemon_async, (void*)manager) != 0) {
        perror("Failed to create thread");
        return 1;
    }

    printf("Server listening on port %d\n", port);

    serve(files);

    destroy_socket_manager(manager);
    free_file(files);

    printf("Bye\n");
    return 0;
}
