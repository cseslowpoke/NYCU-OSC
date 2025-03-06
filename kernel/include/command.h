#ifndef __COMMAND_H
#define __COMMAND_H

typedef struct {
  char *name;
  void (*func)(unsigned int argc, const char *argv[]);
} command_t;

void cmd_help(unsigned int argc, const char *argv[]);

void cmd_hello(unsigned int argc, const char *argv[]);

void cmd_mailbox(unsigned int argc, const char *argv[]);

void cmd_reboot(unsigned int argc, const char *argv[]);

void cmd_ls(unsigned int argc, const char *argv[]);

void cmd_cat(unsigned int argc, const char *argv[]);

#endif
