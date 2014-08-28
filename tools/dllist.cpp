#include "dllist.h"
#include <cstdio>

namespace DL_List {
	List *create(void) {
		List *l = new List;
		l->size = 0;
		l->head = 0;
		l->tail = 0;
		return l;
	}

	void remove(List *list, Node *n) {
		if (list->size == 1) {
			list->head = list->tail = 0;
		}
		else {
			n->prev->next = n->next;
			n->next->prev = n->prev;
		}
		if (n == list->head) {
			list->head = n->next;
			list->tail->next = n->next;
		}
		if (n == list->tail) {
			list->tail = n->prev;
			list->head->prev = n->prev;
		}
		delete n;
		list->size--;
	}

	void add(List *list, Node *n) {
		if (list->head == 0) {
			list->head = n;
			list->tail = n;
			n->next = n;
			n->prev = n;
		}
		else {
			list->tail->next = n;
			n->prev = list->tail;
			n->next = list->head;
			list->head->prev = n;
			list->tail = n;
		}
		list->size++;
	}

	void add_front(List *list, Node *n) {
		add(list, n);
		list->head = n;
		list->tail = n->prev;
	}

	void add_after(List *list, Node *insert_point, Node *n) {
		insert_point->next->prev = n;
		n->next = insert_point->next;
		n->prev = insert_point;
		insert_point->next = n;
		list->size++;
	}

	void dump(List *list) {
		return;
		printf("head=%p tail=%p\n", list->head, list->tail);
		Node *n = list->head;
		do {
			printf("node=%p data=%p\n", n, n->data);
			n = n->next;
		} while (n != list->tail);
	}
} // end namepsace
