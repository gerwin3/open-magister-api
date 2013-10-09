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
typedef struct
{
	struct llist_node* head;
	struct llist_node* tail;
}
llist_t;

struct llist_node
{
	void* v;
	struct llist_node* next;
	struct llist_node* previous;
};

llist_t ll_create ();
void ll_free (llist_t* l);

struct llist_node* ll_push (llist_t* l, void* v);
int ll_comp_size (llist_t* l);

struct llist_node* ll_node_push (llist_t* l, struct llist_node* n, void* v);
void* ll_node_pop (llist_t* l, struct llist_node* n);

#endif