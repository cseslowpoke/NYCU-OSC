#include "fs/fdt.h"
#include "common/string.h"
#include "drivers/uart.h"
#include "mm/mm.h"
#include "mm/simple_alloc.h"
#include "common/utils.h"

static fdt_header *header;
static char *dt_strings;
static uint32_t *dt_struct;

dtb_node_t *parse_fdt_struct() {
  dtb_node_t *node = simple_alloc(sizeof(dtb_node_t));
  static char name_buf[256];
  while (device_ptr(*dt_struct) == FDT_NOP) {
    dt_struct++;
  }
  int name_len = 0;
  if (device_ptr(*dt_struct) == FDT_BEGIN_NODE) {
    dt_struct++;
    while ((*(char *)dt_struct) != '\0') {
      name_buf[name_len++] = *((char *)dt_struct);
      dt_struct = (uint32_t *)((char *)dt_struct + 1);
    }
    name_buf[name_len] = '\0';
    node->name = simple_alloc(name_len + 1);
    node->name = strcpy(node->name, name_buf);
    dt_struct = (uint32_t *)((char *)dt_struct + 1);
    dt_struct = (uint32_t *)(((uint64_t)dt_struct + 3) & ~3);
  } else {
    return NULL;
  }
  while (device_ptr(*dt_struct) == FDT_NOP) {
    dt_struct++;
  }
  dtb_property_t **prop_now = &node->properties;
  while (device_ptr(*dt_struct) == FDT_PROP) {
    dtb_property_t *prop = simple_alloc(sizeof(dtb_property_t));
    *prop_now = prop;
    dt_struct++;
    uint32_t len = device_ptr(*dt_struct);
    dt_struct++;
    uint32_t nameoff = device_ptr(*dt_struct);
    dt_struct++;

    char *prop_name = (char *)dt_strings + nameoff;
    prop->name = prop_name;
    prop->length = len;
    void *value_buf = simple_alloc(len);
    prop->value = value_buf;

    while (len > 0) {
      *((char *)value_buf++) = *(char *)dt_struct;
      dt_struct = (uint32_t *)((char *)dt_struct + 1);
      len--;
    }
    dt_struct = (uint32_t *)(((uint64_t)dt_struct + 3) & ~3);
    prop_now = &(prop->next);
  }
  prop_now = NULL;
  dtb_node_t **child_now = &node->child;
  while (device_ptr(*dt_struct) == FDT_BEGIN_NODE) {
    *child_now = parse_fdt_struct();
    child_now = &((*child_now)->sibling);
    while (device_ptr(*dt_struct) == FDT_NOP) {
      dt_struct++;
    }
  }
  while (device_ptr(*dt_struct) == FDT_NOP) {
    dt_struct++;
  }
  if (device_ptr(*dt_struct) == FDT_END_NODE) {
    dt_struct++;
  } else {
    return NULL;
  }
  return node;
}

void fdt_init(void *fdt_base_address) {
  fdt_base_address = fdt_base_address + VM_BASE;
  header = (fdt_header *)fdt_base_address ;
  if (header->magic != 0xedfe0dd0) {
    uart_send_string("Invalid FDT magic number\r\n");
    return;
  }
  dt_struct = (uint32_t *)((char *)fdt_base_address +
                           device_ptr(header->off_dt_struct));
  dt_strings = ((char *)fdt_base_address + device_ptr(header->off_dt_strings));
  dtb_root = parse_fdt_struct();
  if (dtb_root == NULL) {
    uart_send_string("Failed to parse FDT\r\n");
  }
  void *dtb_end =
      (void *)((char *)fdt_base_address + device_ptr(header->totalsize));
  mm_reserve_region((uint64_t)fdt_base_address, (uint64_t)dtb_end);
}

dtb_node_t *dtb_find_node(char **name_list) {
  dtb_node_t *node = dtb_root->child;
  while (*name_list != NULL) {
    while (node != NULL) {
      if (strcmp(node->name, *name_list) == 0) {
        break;
      }
      node = node->sibling;
    }
    if (node == NULL) {
      return NULL;
    }
    name_list++;
    if (*name_list == NULL) {
      break;
    }
    node = node->child;
  }
  return node;
}

void *dtb_find_property(dtb_node_t *node, const char *name) {
  dtb_property_t *prop = node->properties;
  while (prop != NULL) {
    if (strcmp(prop->name, name) == 0) {
      return prop->value;
    }
    prop = prop->next;
  }
  return NULL;
}
