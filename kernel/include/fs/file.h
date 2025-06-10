#ifndef __FILE_H
#define __FILE_H

#include "common/types.h"
#include "fs/vfs.h"

#define MAX_FILE_NUM 100

/*
 * struct file_entry - file entry in the file table.
 * @name: name of the file.
 * @content: content of the file.
 * @size: size of the file.
 */
typedef struct {
  char *name;
  char *content;
  int size;
} file_entry;

/*
 * NOTE: Current implementation is O(n) every time we search for a file.
 * We can improve this by using a hash table.
 */
extern file_entry file_table[MAX_FILE_NUM];
extern unsigned int file_entry_count;

/*
 * file_register - register a file in the file table.
 * @param filename - name of the file.
 * @param content - content of the file.
 * @param size - size of the file.
 */
void file_register(char *filename, char *content, int size);

/*
 * file_find - find a file in the file table.
 * @param filename - name of the file.
 * @param buf - buffer to store the content of the file.
 */
int file_find(const char *filename, char **buf);

/*
 * file_list - using callback to list all file names in the file table.
 * @param callback - callback function to list the file name.
 */
void file_list(void (*callback)(const char *));

#endif