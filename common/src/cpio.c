#include "cpio.h"
#include "file.h"
#include "string.h"
#include "utils.h"

void cpio_init() {
  volatile cpio_newc_header *header =
      (volatile cpio_newc_header *)CPIO_BASE_ADDR;
  while (1) {
    if (header->c_magic[0] != '0' || header->c_magic[1] != '7' ||
        header->c_magic[2] != '0' || header->c_magic[3] != '7' ||
        header->c_magic[4] != '0' || header->c_magic[5] != '1') {
      break;
    }
    char *file_name = (char *)(header + 1);
    unsigned int file_size = hex2uint((char *)header->c_filesize, 8);
    unsigned int name_size = hex2uint((char *)header->c_namesize, 8);

    int file_name_offset = (name_size + sizeof(cpio_newc_header) + 3) & (~3);
    char *file_content = (char *)header + file_name_offset;

    int file_offset = (file_name_offset + file_size + 3) & (~3);
    header = (cpio_newc_header *)((char *)header + file_offset);

    if (strcmp(file_name, "TRAILER!!!") == 0) {
      break;
    }
    file_register(file_name, file_content, file_size);
  }
}
