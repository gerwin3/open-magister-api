#include "list.h"

llist_t ll_create ()
{
	llist_t ret =
		{ NULL, NULL };

	return ret;
}

void ll_free (llist_t* l)
{
	while (l->head != NULL) {
		ll_node_pop (l, l->head);
	}
}

struct llist_node* ll_push (llist_t* l, void* v)
{
	if (l->head == NULL)
	{
		/* create new node; fill in references
		 * to next/previous */
		struct llist_node* n =
			(struct llist_node*) malloc (sizeof (struct llist_node));

		n->v = v;
		n->previous = NULL;
		n->next = NULL;

		/* push into list */
		l->head = l->tail = n;
	}
	else {
		/* use the push function, will update
		 * list variables for us */
		ll_node_push (l, l->tail, v);
	}

	return l->tail;
}

int ll_comp_size (llist_t* l)
{
	int ret = 0;
	struct llist_node* n = NULL;

	/* simply loop over items and count until we
	 * find the end of the list */
	for (n = l->head; n->next != NULL; n = n->next) {
		ret++;
	}

	return ret;
}

struct llist_node* ll_node_push (llist_t* l, struct llist_node* n, void* v)
{
	/* create new node; fill in next/previous */
	struct llist_node* pushn =
		(struct llist_node*) malloc (sizeof (struct llist_node));

	pushn->v = v;
	pushn->previous = n;
	pushn->next = n->next;

	/* correct n->next->previous! */
	if (n->next != NULL) {
		n->next->previous = pushn;
	}

	/* push into list */
	n->next = pushn;

	/* correct list values */
	if (l->tail == n) {
		l->tail = pushn;
	}

	return pushn;
}

void* ll_node_pop (llist_t* l, struct llist_node* n)
{
	void* v = n->v;

	/* correct references in next/previous */
	if (n->previous != NULL) {
		n->previous->next = n->next;
	}

	if (n->previous != NULL) {
		n->next->previous = n->previous;
	}

	/* correct list values */
	if (l->head == n) {
		l->head = n->next;
	}

	if (l->tail == n) {
		l->tail = n->previous;
	}

	/* release resources */
	free (n);

	return v;
}