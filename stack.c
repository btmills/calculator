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

void stackInit(Stack *s)
{
	s->content = NULL;
	s->size = 0;
	s->top = -1;
}

void stackPush(Stack *s, void* val)
{
	/*if(s->content == NULL) // Initialize
	{
		s->top = -1;
		s->size = 1;
		printf("Allocating stack, size = %d\n", s->size);
		s->content = (void**)realloc(s->content, s->size * sizeof(void*));
	}
	else */if(s->top + 1 >= s->size) // If stack is full
	{
		(s->size)++;
		//printf("Enlarging stack, size = %d\n", s->size);
		s->content = (void**)realloc(s->content, s->size * sizeof(void*));
	}

	(s->top)++;
	//printf("Adding to stack at position %d.", s->top);
	s->content[s->top] = val;
	//printf(" Value successfully added.\n");
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
	//printf("stackPop({size = %d, top = %d})\n", s->size, s->top);
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
	s->size = 0;
	s->top = -1;
}

void stackReverse(Stack *s)
{
	Stack reversed;
	stackInit(&reversed);
	while(stackSize(s) > 0)
	{
		stackPush(&reversed, stackPop(s));
	}
	stackFree(s);
	s->top = reversed.top;
	s->size = reversed.size;
	s->content = reversed.content;
}