#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

/*
 *	llist_t
 *	-------
 *	 => head - ptr to first node
 *	 => tail - ptr to last node
 *
 *	- example:
 *		struct llist_node* n;
 *		for (n = l->head; n->next != NULL; n = n->next) { ... }
 */
typedef struct llist_t
{
	struct llist_node* head;
	struct llist_node* tail;
};

struct llist_node
{
	void* v;
	struct llist_node* next;
	struct llist_node* previous;
};

extern llist_t ll_create ();
extern void ll_free (llist_t* l);

extern struct llist_node* ll_push (llist_t* l, void* v);
extern int ll_comp_size (llist_t* l);

extern struct llist_node* ll_node_push (llist_t* l, llist_node* n, void* v);
extern void* ll_node_pop (llist_t* l, llist_node* n);

#endif