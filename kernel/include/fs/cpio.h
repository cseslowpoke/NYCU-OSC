#ifndef __CPIO_H
#define __CPIO_H

/*
 * cpio_newc_header - representation of a newc header in a cpio archive.
 * more information can be found at man page 5.
 * https://man.freebsd.org/cgi/man.cgi?query=cpio&sektion=5.
 */
typedef struct {
  char c_magic[6];
  char c_ino[8];
  char c_mode[8];
  char c_uid[8];
  char c_gid[8];
  char c_nlink[8];
  char c_mtime[8];
  char c_filesize[8];
  char c_devmajor[8];
  char c_devminor[8];
  char c_rdevmajor[8];
  char c_rdevminor[8];
  char c_namesize[8];
  char c_check[8];
} cpio_newc_header;

/*
 * cpio_init - initialize the cpio archive.
 * get the start address from dtb and store file to file_table in file.h.
 */
void cpio_init();

#endif // __CPIO_H
