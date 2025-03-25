#ifndef __FDT_H
#define __FDT_H

#include "common/types.h"

#define FDT_BEGIN_NODE (0x00000001)
#define FDT_END_NODE (0x00000002)
#define FDT_PROP (0x00000003)
#define FDT_NOP (0x00000004)
#define FDT_END (0x00000009)

/*
 * fdt_header - representation of the header of a flattened device tree.
 * more information can be found at
 * https://devicetree-specification.readthedocs.io/en/stable/flattened-format.html
 */
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

/*
 * fdt_reserve_entry - representation of a reserved memory region.
 * not using in now, but may be useful in the future.
 */
typedef struct {
  uint64_t address;
  uint64_t size;
} fdt_reserve_entry;

/*
 * dtb_property - representation of a property in a device tree node.
 * @name: name of the property.
 * @value: value of the property.
 * @length: length of the value.
 * @next: pointer to the next property in same node.
 */
typedef struct dtb_property {
  const char *name;
  void *value;
  unsigned int length;
  struct dtb_property *next;
} dtb_property_t;

/*
 * dtb_node - representation of a node in a device tree.
 * @name: name of the node.
 * @properties: properties of the node.
 * @child: pointer to the first child node.
 * @child: pointer to the sibling node.
 */
typedef struct dtb_node {
  char *name;
  dtb_property_t *properties;
  struct dtb_node *child;
  struct dtb_node *sibling;
} dtb_node_t;

static dtb_node_t *dtb_root;
static void *dtb_strings_base;
static void *dtb_struct_base;

/*
 * fdt_init - parsing the device tree blob and build the device tree.
 * @param fdt_base_address - base address of the device tree blob.
 */
void fdt_init(void *fdt_base_address);

/*
 * dtb_find_node - find a node in the device tree.
 * @param name_list - list of names of the node.
 *
 * before using this function, you need to split the path of the node into a
 * list.
 * e.g. /soc/uart@101f1000 -> ["soc", "uart@101f1000", NULL]
 */
dtb_node_t *dtb_find_node(char **name_list);

/*
 * dtb_find_property - find a property in a node.
 * @param node - node to search.
 * @param name - name of the property.
 *
 */
void *dtb_find_property(dtb_node_t *node, const char *name);

/*
 * NOTE: should be better in future.
 * device_ptr - convert a big-endian presentation to little-endian.
 * @param x - big-endian presentation.
 */
#define device_ptr(x)                                                          \
  ((((x) >> 24) & 0x000000FF) | (((x) >> 8) & 0x0000FF00) |                    \
   (((x) << 8) & 0x00FF0000) | (((x) << 24) & 0xFF000000))

#endif // __FDT_H
