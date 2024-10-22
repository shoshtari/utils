#include "file_utils.h"

#define MAX_FILE_SIZE 32768000

char *realloc_pointer(char *a, int n) {
  char *ans = (char *)malloc(n * 2);
  for (int i = 0; i < n; i++) {
    ans[i] = a[i];
  }

  free(a);
  return ans;
}

char *get_file_path(char *dir, char *name) {
  char *path = (char *)malloc(strlen(dir) + strlen(name) + 100);
  strcpy(path, dir);
  strcat(path, "/");
  strcat(path, name);
  return path;
}

int read_file(char *dir, char *name, fileinfo *res, int load_data) {
  res->name = malloc(strlen(name) + 1);
  strcpy(res->name, name);

  char *path = get_file_path(dir, name);
  struct stat filestat;
  if (stat(path, &filestat) < 0) {
    perror("error on stat file");
  }

  res->size = filestat.st_size;

  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    perror("error on opening file");
    return -1;
  }
  res->fd = fd;

  if (load_data) {
    res->data = (char *)malloc(filestat.st_size + 1);
    if (read(fd, res->data, filestat.st_size + 1) != filestat.st_size) {
      perror("file size mismatch");
    }
  res->hash = calculate_md5(res->data, res->size);
  }
  else{
	  res->hash = "server lazy load is active, can't calculate md5";
  }
  free(path);

  return 0;
}

int read_chunk(int fd, char *data, int length, int offset) {
  return pread(fd, data, length, offset);
}

int read_files(char *dir_path, dir_files *result, int load_data) {
  DIR *dr = opendir(dir_path);

  if (dr == NULL) {
    perror("Could not open files directory");
    return -1;
  }

  dir_files ans;

  int capacity = 1;
  result->filecounts = 0;
  result->files = (fileinfo *)malloc(sizeof(fileinfo));

  struct dirent *de;
  while ((de = readdir(dr)) != NULL) {
    if (de->d_type != DT_REG) {
      continue;
    }
    if (strlen(de->d_name) > FILENAME_MAX || strchr(de->d_name, ';') ||
        strchr(de->d_name, '@')) {
      printf("file %s name is not valid, ignoring it\n", de->d_name);
      continue;
    }

    if (read_file(dir_path, de->d_name, &result->files[result->filecounts],
                  load_data) < 0) {
      perror("can't read file\n");
    }

    result->filecounts++;
    if (result->filecounts == capacity) {
      result->files = (fileinfo *)realloc_pointer(
          (char *)result->files, result->filecounts * sizeof(fileinfo));
      capacity *= 2;
    }
  }

  closedir(dr);
  return 0;
}

char *gen_file_list_entry(dir_files files, int index) {
  if (index >= files.filecounts) {
    printf("invalid file request\n");
    return NULL;
  }

  char *ans = (char *)malloc(BUFSIZ);
  if (ans == NULL) {
    printf("malloc error");
    return NULL;
  }

  fileinfo file = files.files[index];
  int cnt = (snprintf(ans, 10, "%d@%s@%d@%s;", index, file.name, file.size,
                      file.hash));
  if (cnt < 0) {
    printf("snprintf error\n");
    return NULL;
  }

  free(ans);
  ans = (char *)malloc(cnt + 1e5);

  int cnt2 = (snprintf(ans, cnt + 1, "%d@%s@%d@%s;", index, file.name,
                       file.size, file.hash));
  if (cnt < 0 || cnt2 != cnt) {
    printf("unexpected error on snprintf");
    return NULL;
  }
  return ans;
}

int write_to_file(char *name, char *data, int n) {
  int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);

  if (fd == -1) {
    perror("Error opening file");
    return -1;
  }

  int bytes_written = write(fd, data, n);

  if (bytes_written == -1 || bytes_written < n) {
    perror("Error writing to file");
    close(fd);
    return -1;
  }

  if (close(fd) == -1) {
    perror("Error closing file");
    return -1;
  }

  return bytes_written;
}

void free_file(dir_files files) {
  for (int i = 0; i < files.filecounts; i++) {
    fileinfo file = files.files[i];
    free(file.name);
    free(file.hash);
    free(file.data);
    if (file.fd < 0) {
      close(file.fd);
    }
  }
  free(files.files);
}
