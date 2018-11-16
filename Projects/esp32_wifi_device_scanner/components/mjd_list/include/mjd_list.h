/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _MJD_LINUX_LIST_H
#define _MJD_LINUX_LIST_H

#include <stdbool.h>
#include <stddef.h>

#include "linux_const.h"
#include "linux_poison.h"


/**********
 *  Header stuff copied from includes
 *
 */

/*
 *  @source include/linux/compiler_types.h
 */
/* Are two types/vars the same type (ignoring qualifiers)? */
#ifndef __same_type
# define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#endif

/*
 *  @source include/linux/kernel.h
 */
/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 *
 */
#ifndef container_of
#define container_of(ptr, type, member) ({              \
    void *__mptr = (void *)(ptr);                   \
    ((type *)(__mptr - offsetof(type, member))); })
#endif

/*
 * @source tools/include/types.h
 */
struct mjd_list_head {
    struct mjd_list_head *next, *prev;
};


/**********
 * CUSTOM LOGIC ADDED BY ME
 */
/*
 * mjd_list_count - count the items in the linked list
 * @important A custom method added by me.
 * @head:     ptr to the head of your list.
 * @varcount  prt to uint32_t variable
 *
 */
#define mjd_list_count(head, varcount) \
    { \
        *varcount = 0;  \
        struct mjd_list_head *cursor; \
        for (cursor = (head)->next; cursor != (head); cursor = cursor->next) { \
            ++(*varcount); \
        } \
    }


/**********
 * MAIN
 */
/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

#define MJD_LIST_HEAD_INIT(name) { &(name), &(name) }

#define MJD_LIST_HEAD(name) \
    struct mjd_list_head name = MJD_LIST_HEAD_INIT(name)

static inline void MJD_INIT_LIST_HEAD(struct mjd_list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline bool __mjd_list_add_valid(struct mjd_list_head *new,
                struct mjd_list_head *prev,
                struct mjd_list_head *next)
{
    return true;
}
static inline bool __mjd_list_del_entry_valid(struct mjd_list_head *entry)
{
    return true;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __mjd_list_add(struct mjd_list_head *new,
                  struct mjd_list_head *prev,
                  struct mjd_list_head *next)
{
    if (!__mjd_list_add_valid(new, prev, next))
        return;

    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/**
 * mjd_list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void mjd_list_add(struct mjd_list_head *new, struct mjd_list_head *head)
{
    __mjd_list_add(new, head, head->next);
}


/**
 * mjd_list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void mjd_list_add_tail(struct mjd_list_head *new, struct mjd_list_head *head)
{
    __mjd_list_add(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __mjd_list_del(struct mjd_list_head * prev, struct mjd_list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * mjd_list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: mjd_list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void __mjd_list_del_entry(struct mjd_list_head *entry)
{
    if (!__mjd_list_del_entry_valid(entry))
        return;

    __mjd_list_del(entry->prev, entry->next);
}

static inline void mjd_list_del(struct mjd_list_head *entry)
{
    __mjd_list_del_entry(entry);
    entry->next = LIST_POISON1;
    entry->prev = LIST_POISON2;
}

/**
 * mjd_list_replace - replace old entry by new one
 * @old : the element to be replaced
 * @new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void mjd_list_replace(struct mjd_list_head *old,
                struct mjd_list_head *new)
{
    new->next = old->next;
    new->next->prev = new;
    new->prev = old->prev;
    new->prev->next = new;
}

static inline void mjd_list_replace_init(struct mjd_list_head *old,
                    struct mjd_list_head *new)
{
    mjd_list_replace(old, new);
    MJD_INIT_LIST_HEAD(old);
}

/**
 * mjd_list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void mjd_list_del_init(struct mjd_list_head *entry)
{
    __mjd_list_del_entry(entry);
    MJD_INIT_LIST_HEAD(entry);
}

/**
 * mjd_list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void mjd_list_move(struct mjd_list_head *list, struct mjd_list_head *head)
{
    __mjd_list_del_entry(list);
    mjd_list_add(list, head);
}

/**
 * mjd_list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void mjd_list_move_tail(struct mjd_list_head *list,
                  struct mjd_list_head *head)
{
    __mjd_list_del_entry(list);
    mjd_list_add_tail(list, head);
}

/**
 * mjd_list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int mjd_list_is_last(const struct mjd_list_head *list,
                const struct mjd_list_head *head)
{
    return list->next == head;
}

/**
 * mjd_list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int mjd_list_empty(const struct mjd_list_head *head)
{
    return head->next == head;
}

/**
 * mjd_list_empty_careful - tests whether a list is empty and not being modified
 * @head: the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using mjd_list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is mjd_list_del_init(). Eg. it cannot be used
 * if another CPU could re-mjd_list_add() it.
 */
static inline int mjd_list_empty_careful(const struct mjd_list_head *head)
{
    struct mjd_list_head *next = head->next;
    return (next == head) && (next == head->prev);
}

/**
 * mjd_list_rotate_left - rotate the list to the left
 * @head: the head of the list
 */
static inline void mjd_list_rotate_left(struct mjd_list_head *head)
{
    struct mjd_list_head *first;

    if (!mjd_list_empty(head)) {
        first = head->next;
        mjd_list_move_tail(first, head);
    }
}

/**
 * mjd_list_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static inline int mjd_list_is_singular(const struct mjd_list_head *head)
{
    return !mjd_list_empty(head) && (head->next == head->prev);
}

static inline void __mjd_list_cut_position(struct mjd_list_head *list,
        struct mjd_list_head *head, struct mjd_list_head *entry)
{
    struct mjd_list_head *new_first = entry->next;
    list->next = head->next;
    list->next->prev = list;
    list->prev = entry;
    entry->next = list;
    head->next = new_first;
    new_first->prev = head;
}

/**
 * mjd_list_cut_position - cut a list into two
 * @list: a new list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *  and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static inline void mjd_list_cut_position(struct mjd_list_head *list,
        struct mjd_list_head *head, struct mjd_list_head *entry)
{
    if (mjd_list_empty(head))
        return;
    if (mjd_list_is_singular(head) &&
        (head->next != entry && head != entry))
        return;
    if (entry == head)
        MJD_INIT_LIST_HEAD(list);
    else
        __mjd_list_cut_position(list, head, entry);
}

static inline void __mjd_list_splice(const struct mjd_list_head *list,
                 struct mjd_list_head *prev,
                 struct mjd_list_head *next)
{
    struct mjd_list_head *first = list->next;
    struct mjd_list_head *last = list->prev;

    first->prev = prev;
    prev->next = first;

    last->next = next;
    next->prev = last;
}

/**
 * mjd_list_splice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void mjd_list_splice(const struct mjd_list_head *list,
                struct mjd_list_head *head)
{
    if (!mjd_list_empty(list))
        __mjd_list_splice(list, head, head->next);
}

/**
 * mjd_list_splice_tail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void mjd_list_splice_tail(struct mjd_list_head *list,
                struct mjd_list_head *head)
{
    if (!mjd_list_empty(list))
        __mjd_list_splice(list, head->prev, head);
}

/**
 * mjd_list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void mjd_list_splice_init(struct mjd_list_head *list,
                    struct mjd_list_head *head)
{
    if (!mjd_list_empty(list)) {
        __mjd_list_splice(list, head, head->next);
        MJD_INIT_LIST_HEAD(list);
    }
}

/**
 * mjd_list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static inline void mjd_list_splice_tail_init(struct mjd_list_head *list,
                     struct mjd_list_head *head)
{
    if (!mjd_list_empty(list)) {
        __mjd_list_splice(list, head->prev, head);
        MJD_INIT_LIST_HEAD(list);
    }
}

/**
 * mjd_list_entry - get the struct for this entry
 * @ptr:    the &struct mjd_list_head pointer.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the mjd_list_head within the struct.
 */
#define mjd_list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * mjd_list_first_entry - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the mjd_list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define mjd_list_first_entry(ptr, type, member) \
    mjd_list_entry((ptr)->next, type, member)

/**
 * mjd_list_last_entry - get the last element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the mjd_list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define mjd_list_last_entry(ptr, type, member) \
    mjd_list_entry((ptr)->prev, type, member)

/**
 * mjd_list_first_entry_or_null - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the mjd_list_head within the struct.
 *
 * Note that if the list is empty, it returns NULL.
 */
#define mjd_list_first_entry_or_null(ptr, type, member) ({ \
    struct mjd_list_head *head__ = (ptr); \
    struct mjd_list_head *pos__ = head__->next; \
    pos__ != head__ ? mjd_list_entry(pos__, type, member) : NULL; \
})

/**
 * mjd_list_next_entry - get the next element in list
 * @pos:    the type * to cursor
 * @member: the name of the mjd_list_head within the struct.
 */
#define mjd_list_next_entry(pos, member) \
    mjd_list_entry((pos)->member.next, typeof(*(pos)), member)

/**
 * mjd_list_prev_entry - get the prev element in list
 * @pos:    the type * to cursor
 * @member: the name of the mjd_list_head within the struct.
 */
#define mjd_list_prev_entry(pos, member) \
    mjd_list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
 * mjd_list_for_each    -   iterate over a list
 * @pos:    the &struct mjd_list_head to use as a loop cursor.
 * @head:   the head for your list.
 */
#define mjd_list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * mjd_list_for_each_prev   -   iterate over a list backwards
 * @pos:    the &struct mjd_list_head to use as a loop cursor.
 * @head:   the head for your list.
 */
#define mjd_list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * mjd_list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:    the &struct mjd_list_head to use as a loop cursor.
 * @n:      another &struct mjd_list_head to use as temporary storage
 * @head:   the head for your list.
 */
#define mjd_list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
        pos = n, n = pos->next)

/**
 * mjd_list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:    the &struct mjd_list_head to use as a loop cursor.
 * @n:      another &struct mjd_list_head to use as temporary storage
 * @head:   the head for your list.
 */
#define mjd_list_for_each_prev_safe(pos, n, head) \
    for (pos = (head)->prev, n = pos->prev; \
         pos != (head); \
         pos = n, n = pos->prev)

/**
 * mjd_list_for_each_entry  -   iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the mjd_list_head within the struct.
 */
#define mjd_list_for_each_entry(pos, head, member)              \
    for (pos = mjd_list_first_entry(head, typeof(*pos), member);    \
         &pos->member != (head);                    \
         pos = mjd_list_next_entry(pos, member))

/**
 * mjd_list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the mjd_list_head within the struct.
 */
#define mjd_list_for_each_entry_reverse(pos, head, member)          \
    for (pos = mjd_list_last_entry(head, typeof(*pos), member);     \
         &pos->member != (head);                    \
         pos = mjd_list_prev_entry(pos, member))

/**
 * mjd_list_prepare_entry - prepare a pos entry for use in mjd_list_for_each_entry_continue()
 * @pos:    the type * to use as a start point
 * @head:   the head of the list
 * @member: the name of the mjd_list_head within the struct.
 *
 * Prepares a pos entry for use as a start point in mjd_list_for_each_entry_continue().
 */
#define mjd_list_prepare_entry(pos, head, member) \
    ((pos) ? : mjd_list_entry(head, typeof(*pos), member))

/**
 * mjd_list_for_each_entry_continue - continue iteration over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the mjd_list_head within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define mjd_list_for_each_entry_continue(pos, head, member)         \
    for (pos = mjd_list_next_entry(pos, member);            \
         &pos->member != (head);                    \
         pos = mjd_list_next_entry(pos, member))

/**
 * mjd_list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the mjd_list_head within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define mjd_list_for_each_entry_continue_reverse(pos, head, member)     \
    for (pos = mjd_list_prev_entry(pos, member);            \
         &pos->member != (head);                    \
         pos = mjd_list_prev_entry(pos, member))

/**
 * mjd_list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the mjd_list_head within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define mjd_list_for_each_entry_from(pos, head, member)             \
    for (; &pos->member != (head);                  \
         pos = mjd_list_next_entry(pos, member))

/**
 * mjd_list_for_each_entry_from_reverse - iterate backwards over list of given type
 *                                    from the current point
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the mjd_list_head within the struct.
 *
 * Iterate backwards over list of given type, continuing from current position.
 */
#define mjd_list_for_each_entry_from_reverse(pos, head, member)     \
    for (; &pos->member != (head);                  \
         pos = mjd_list_prev_entry(pos, member))

/**
 * mjd_list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the mjd_list_head within the struct.
 */
#define mjd_list_for_each_entry_safe(pos, n, head, member)          \
    for (pos = mjd_list_first_entry(head, typeof(*pos), member),    \
        n = mjd_list_next_entry(pos, member);           \
         &pos->member != (head);                    \
         pos = n, n = mjd_list_next_entry(n, member))

/**
 * mjd_list_for_each_entry_safe_continue - continue list iteration safe against removal
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the mjd_list_head within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define mjd_list_for_each_entry_safe_continue(pos, n, head, member)         \
    for (pos = mjd_list_next_entry(pos, member),                \
        n = mjd_list_next_entry(pos, member);               \
         &pos->member != (head);                        \
         pos = n, n = mjd_list_next_entry(n, member))

/**
 * mjd_list_for_each_entry_safe_from - iterate over list from current point safe against removal
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the mjd_list_head within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define mjd_list_for_each_entry_safe_from(pos, n, head, member)             \
    for (n = mjd_list_next_entry(pos, member);                  \
         &pos->member != (head);                        \
         pos = n, n = mjd_list_next_entry(n, member))

/**
 * mjd_list_for_each_entry_safe_reverse - iterate backwards over list safe against removal
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the mjd_list_head within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define mjd_list_for_each_entry_safe_reverse(pos, n, head, member)      \
    for (pos = mjd_list_last_entry(head, typeof(*pos), member),     \
        n = mjd_list_prev_entry(pos, member);           \
         &pos->member != (head);                    \
         pos = n, n = mjd_list_prev_entry(n, member))

/**
 * mjd_list_safe_reset_next - reset a stale mjd_list_for_each_entry_safe loop
 * @pos:    the loop cursor used in the mjd_list_for_each_entry_safe loop
 * @n:      temporary storage used in mjd_list_for_each_entry_safe
 * @member: the name of the mjd_list_head within the struct.
 *
 * mjd_list_safe_reset_next is not safe to use in general if the list may be
 * modified concurrently (eg. the lock is dropped in the loop body). An
 * exception to this is if the cursor element (pos) is pinned in the list,
 * and mjd_list_safe_reset_next is called after re-taking the lock and before
 * completing the current iteration of the loop body.
 */
#define mjd_list_safe_reset_next(pos, n, member)                \
    n = mjd_list_next_entry(pos, member)


#endif /*_MJD_LINUX_LIST_H*/
