#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

/*
typedef struct
{
	void *content;
	int size;
	int top;
} Stack;
*/

void stackPush(Stack *s, void* val)
{
	if(s->content == NULL) // Initialize
	{
		s->top = -1;
		s->size = 0;
		s->content = (void**)malloc(++(s->size) * sizeof(void*));
		//printf("Allocated stack, size = %d\n", s->size);
	}
	else if(s->top + 1 >= s->size) // If stack is full
	{
		s->content = (void**)realloc(s->content, ++(s->size) * sizeof(void*));
		//printf("Enlarged stack, size = %d\n", s->size);
	}

	(s->top)++;
	//printf("Adding to stack at position %d\n", s->top);
	s->content[s->top] = val;
}

void* stackTop(Stack *s)
{
	//printf("stackTop({size = %d, top = %d})\n", s->size, s->top);
	void *ret = NULL;
	if(s->top >= 0 && s->content != NULL)
		ret = s->content[s->top];
	return ret;
}

void* stackPop(Stack *s)
{
	void *ret = NULL;
	if(s->top >= 0 && s->content != NULL)
		ret = s->content[(s->top)--];
	return ret;
}

int stackSize(Stack *s)
{
	return s->top + 1;
}

void stackFree(Stack *s)
{
	free(s->content);
	s->content = NULL;
}