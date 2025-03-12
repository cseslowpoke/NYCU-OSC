#ifndef __FILE_H
#define __FILE_H

#define MAX_FILE_NUM 100

typedef struct {
  char *name;
  char *content;
  int size;
} file_entry;

extern file_entry file_table[MAX_FILE_NUM];
extern unsigned int file_entry_count;

void file_register(char *filename, char *content, int size);

char *file_find(const char *filename);

void file_list(void (*callback)(const char *));

#endif
