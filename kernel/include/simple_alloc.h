#ifndef __SIMPLE_ALLOC_H
#define __SIMPLE_ALLOC_H

extern char _heap_start;

static volatile char *heap_start;
static volatile char *heap_end;
static char *heap_current;

void *simple_alloc(unsigned int size);

void simple_alloc_init();

#endif
