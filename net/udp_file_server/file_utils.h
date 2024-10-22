
#ifndef UTILS_INCLUDE
#define UTILS_INCLUDE 

#include "md5.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>

#include "md5.h"
#include "net/socket_manager.h"
typedef struct fileinfo {
	char* name;
	char* data;
	char* hash;
	int size;
	int fd;
}fileinfo;

typedef struct dir_files {
  int filecounts;
  fileinfo* files;
} dir_files;




char *realloc_pointer(char *a, int n) ;
int read_files(char *dir_path, dir_files *result, int load_data) ;
int read_chunk(int fd, char *data, int length, int offset) ;
int write_to_file(char *name, char *data, int n) ;
char *gen_file_list_entry(dir_files files, int index) ;
void free_file(dir_files files);

#endif



