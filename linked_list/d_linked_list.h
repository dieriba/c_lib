#ifndef __D_LINKED_LIST__H
#define __D_LINKED_LIST__H

typedef struct _DSinglyList DSinglyList;
typedef struct _DDoubleList DDoubleList;

struct _DSinglyList {
	void*			data;
	DSinglyList*	next;
};

struct _DDoubleList {
	void*			data;
	DDoubleList*	next;
	DDoubleList*	prev;
};

#endif