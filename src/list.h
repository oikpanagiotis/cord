#ifndef LIST_H
#define LIST_H

typedef struct _node {
	void *data;	
	struct _node *next;
	struct _node *prev;
} _node;

typedef void *(*construct_item)(void);
typedef void (*destruct_item)(void *self);

typedef struct list_t {
	_node *head;
	_node *tail;
	int num_items;
	construct_item constructor_cb;
	destruct_item destructor_cb;
} list_t;


list_t *list_create();
void list_destroy(list_t *l);

// Returns -1 on failure or 0 on success
int list_push(list_t *l, void *item);


#endif
