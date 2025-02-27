#ifndef __SHELL_H
#define __SHELL_H

#define SHELL_BUFFER_SIZE 128

typedef struct {
  char *name;
  void (*func)(void);
} command_t;

void shell_start();

void shell_process_command(char *buf, int buf_len);
#endif // __SHELL_H
