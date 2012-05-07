#include <stdio.h>
#include "stack.h"

int main()
{
	int i;
	Stack strs;
	char* val = NULL;

	val = "this";
	printf("Pushing \"%s\"\n", val);
	stackPush(&strs, val);
	printf("Top of stack: \"%s\"\n", (char*)stackTop(&strs));
	val = "is";
	printf("Pushing \"%s\"\n", val);
	stackPush(&strs, val);
	printf("Top of stack: \"%s\"\n", (char*)stackTop(&strs));
	val = "a";
	printf("Pushing \"%s\"\n", val);
	stackPush(&strs, val);
	printf("Top of stack: \"%s\"\n", (char*)stackTop(&strs));
	val = "stack";
	printf("Pushing \"%s\"\n", val);
	stackPush(&strs, val);
	printf("Top of stack: \"%s\"\n", (char*)stackTop(&strs));
	printf("\n");

	printf("Stack before stackPop():\n");
	for(i = stackSize(&strs) - 1; i >= 0; i--)
	{
		printf("\"%s\"\n", (char*)(strs.content[i]));
	}
	printf("\n");

	val = stackPop(&strs);
	printf("stackPop() returned \"%s\"\n", val);
	printf("Remainder of stack:\n");
	while(stackSize(&strs))
	{
		val = (char*)stackPop(&strs);
		printf("\"%s\"\n", val);
	}
	val = (char*)stackPop(&strs);
	printf("Extra call of stackPop() returned \"%s\"\n", val);

	stackFree(&strs);
}