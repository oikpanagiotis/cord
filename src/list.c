#include "list.h"
#include <stdio.h>
#include <stdlib.h>

static _node *node_create(void *data) {
	_node *n = malloc(sizeof(_node));
	if (!n) {
		fprintf(stderr, "Failed to allocate list node\n");
		return NULL;
	}
	n->next = NULL;
	n->data = data;
	return n;
}

list_t *list_create(void) {
	list_t *l = malloc(sizeof(list_t));
	if (!l) {
		fprintf(stderr, "Failed to allocate list_t\n");
		return NULL;	
	}
	l->num_items = 0;
	l->head = NULL;
	l->tail = NULL;
	return l;
}

void list_destroy(list_t *l) {

}

int list_push(list_t *l, void *item) {
	_node *head = l->head;
	if (!head) {
		head = node_create(item);
		l->num_items++;
		return 0;
	}

	_node *temp = head;
	while (temp->next) {

	}
}
