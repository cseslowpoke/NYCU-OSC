#ifndef __LIST_H
#define __LIST_H

#include "common/utils.h"
typedef struct list_head {
  struct list_head *prev;
  struct list_head *next;
} list_head_t;

#define INIT_LIST_HEAD(ptr)                                                    \
  do {                                                                         \
    (ptr)->next = (ptr);                                                       \
    (ptr)->prev = (ptr);                                                       \
  } while (0)

#define list_entry(ptr, type, member) container_of(ptr, type, member)

/*
 * Insert a new entry between two known consecutive entries.
 *
 * @param new The new entry to be added.
 * @param prev The previous entry in the lists.
 * @param next The next entry in the lists.
 */
static inline void __list_add(list_head_t *new, list_head_t *prev,
                              list_head_t *next) {
  prev->next = new;
  new->prev = prev;
  new->next = next;
  next->prev = new;
}

/*
 * A waraper to add entry after the specified head.
 *
 * @param new The new entry to be added.
 * @param head The head of the list.
 */
static inline void list_add(list_head_t *new, list_head_t *head) {
  __list_add(new, head, head->next);
}

/*
 * A waraper to add entry before the specified head.
 *
 * @param new The new entry to be added.
 * @param head The head of the list.
 */
static inline void list_add_tail(list_head_t *new, list_head_t *head) {
  __list_add(new, head->prev, head);
}

/*
 * Removes an entry from a doubly linked list.
 *
 * @param entry the list entry to remove.
 */
static inline void list_del(list_head_t *entry) {
  entry->prev->next = entry->next;
  entry->next->prev = entry->prev;
  entry->next = entry->prev = entry; // reset entry to single linked list.
}

static inline int list_empty(list_head_t *head) { return head->next == head; }

#define list_for_each(pos, head)                                               \
  for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_entry(pos, head, member)                                 \
  for (pos = list_entry((head)->next, typeof(*pos), member);                   \
       &pos->member != (head);                                                 \
       pos = list_entry(pos->member.next, typeof(*pos), member))

#endif // __LIST_H
