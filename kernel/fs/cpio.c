#include "fs/cpio.h"
#include "common/string.h"
#include "common/types.h"
#include "common/utils.h"
#include "drivers/uart.h"
#include "fs/fdt.h"
#include "fs/file.h"
#include "mm/mm.h"

void cpio_init() {
  char *name_list[2];
  name_list[0] = "chosen";
  name_list[1] = NULL;
  dtb_node_t *chosen_node = dtb_find_node(name_list);
  if (chosen_node == NULL) {
    uart_send_string("Cannot find chosen node in dtb\n");
    return;
  }
  uint32_t *base_addr = (uint32_t *)((uint64_t)device_ptr(
      *(uint32_t *)dtb_find_property(chosen_node, "linux,initrd-start")) + VM_BASE);
  uint32_t *end_addr = (uint32_t *)((uint64_t)device_ptr(
      *(uint32_t *)dtb_find_property(chosen_node, "linux,initrd-end")) + VM_BASE);

  mm_reserve_region((uint64_t)base_addr, (uint64_t)end_addr);

  volatile cpio_newc_header *header = (volatile cpio_newc_header *)base_addr;
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
