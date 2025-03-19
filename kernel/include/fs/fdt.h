#ifndef __FDT_H
#define __FDT_H

#include "common/types.h"

#define FDT_BEGIN_NODE (0x00000001)
#define FDT_END_NODE (0x00000002)
#define FDT_PROP (0x00000003)
#define FDT_NOP (0x00000004)
#define FDT_END (0x00000009)

typedef struct {
  uint32_t magic;
  uint32_t totalsize;
  uint32_t off_dt_struct;
  uint32_t off_dt_strings;
  uint32_t off_mem_rsvmap;
  uint32_t version;
  uint32_t last_comp_version;
  uint32_t boot_cpuid_phys;
  uint32_t size_dt_strings;
  uint32_t size_dt_struct;
} fdt_header;

typedef struct {
  uint64_t address;
  uint64_t size;
} fdt_reserve_entry;

typedef struct dtb_property {
  const char *name;
  void *value;
  unsigned int length;
  struct dtb_property *next;
} dtb_property_t;

typedef struct dtb_node {
  char *name;
  dtb_property_t *properties;
  struct dtb_node *child;
  struct dtb_node *sibling;
} dtb_node_t;

static dtb_node_t *dtb_root;
static void *dtb_strings_base;
static void *dtb_struct_base;

void fdt_init(void *fdt_base_address);

dtb_node_t *dtb_find_node(char **name_list);

void *dtb_find_property(dtb_node_t *node, const char *name);

#define device_ptr(x)                                                          \
  ((((x) >> 24) & 0x000000FF) | (((x) >> 8) & 0x0000FF00) |                    \
   (((x) << 8) & 0x00FF0000) | (((x) << 24) & 0xFF000000))

#endif // __FDT_H
