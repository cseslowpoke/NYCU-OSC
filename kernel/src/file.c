#include "file.h"
#include "string.h"

file_entry file_table[MAX_FILE_NUM];
unsigned int file_entry_count;

void file_register(char *filename, char *content, int size) {
  if (file_entry_count >= MAX_FILE_NUM) {
    return;
  }
  file_table[file_entry_count].name = filename;
  file_table[file_entry_count].content = content;
  file_table[file_entry_count].size = size;
  file_entry_count++;
}

char *file_find(const char *filename) {
  for (int i = 0; i < file_entry_count; i++) {
    if (strcmp(file_table[i].name, filename) == 0) {
      return file_table[i].content;
    }
  }
  return 0;
}

void file_list(void (*callback)(const char *)) {
  for (int i = 0; i < file_entry_count; i++) {
    callback(file_table[i].name);
  }
}
