#ifndef __SIMPLE_ALLOC_H
#define __SIMPLE_ALLOC_H

void *simple_alloc(unsigned int size);

void simple_alloc_init();

void *simple_alloc_begin(); // returns _heap_start
void *simple_alloc_end();   // returns current ptr

#endif
