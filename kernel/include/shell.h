#ifndef __SHELL_H
#define __SHELL_H

#define SHELL_BUFFER_SIZE 128
#define SHELL_MAX_ARGC 16

void shell_start();

void shell_process_command(char *buf, int buf_len);

#endif // __SHELL_H
