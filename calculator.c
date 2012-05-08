#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // Temporary
#include "stack.h"

#define bool char
#define true 1
#define false 0

typedef enum
{
	addop,
	multop,
	expop,
	lparen,
	rparen,
	digit,
	decimal,
	space,
	invalid
} Symbol;

typedef enum
{
	divZero,
	overflow,
	parenMismatch
} Error;

typedef char* token;

typedef double number;

void raise(Error err)
{
	char* msg;
	switch(err)
	{
		case divZero:
			msg = "Divide by zero";
			break;
		case overflow:
			msg = "Overflow";
			break;
		case parenMismatch:
			msg = "Mismatched parentheses";
			break;
	}
	printf("\tError: %s\n", msg);
}

inline unsigned int toDigit(char ch)
{
	return ch - '0';
}

number buildNumber(token str)
{
	number result = 0;
	while(*str && *str != '.')
	{
		result = result * 10 + toDigit(*str++);
	}
	return result;
}

token num2Str(number num)
{
	token str = (token)malloc(20*sizeof(char));
	snprintf(str, 19, "%f", num);
	return str;
}

token doOp(token loperand, token op, token roperand)
{
	number lside = buildNumber(loperand);
	number rside = buildNumber(roperand);
	number ret;
	switch(*op)
	{
		case '^':
			{
				ret = pow(lside, rside);
			}
			break;
		case '*':
			{
				ret = lside * rside;
			}
			break;
		case '/':
			{
				if(rside == 0)
					raise(divZero);
				else
					ret = lside / rside;
			}
			break;
		case '%':
			{
				if(rside == 0)
					raise(divZero);
				else
				{
					ret = (int)(lside / rside);
					ret = lside - (ret * rside);
				}
			}
			break;
		case '+':
			{
				ret = lside + rside;
			}
			break;
		case '-':
			{
				ret = lside - rside;
			}
			break;
	}
	return num2Str(ret);
}

/*
 * Similar to fgets(), but handles automatic reallocation of the buffer.
 * Only parameter is the input stream.
 * Return value is a string. Don't forget to free it.
 */
char* ufgets(FILE* stream)
{
    unsigned int maxlen = 128, size = 128;
    char* buffer = (char*)malloc(maxlen);

    if(buffer != NULL) /* NULL if malloc() fails */
    {
        char ch = EOF;
        int pos = 0;

        /* Read input one character at a time, resizing the buffer as necessary */
        while((ch = getchar()) != EOF && ch != '\n')
        {
            buffer[pos++] = ch;
            if(pos == size) /* Next character to be inserted needs more memory */
            {
                size = pos + maxlen;
                buffer = (char*)realloc(buffer, size);
            }
        }
        buffer[pos] = '\0'; /* Null-terminate the completed string */
    }
    return buffer;
}

Symbol type(char ch)
{
	Symbol result;
	switch(ch)
	{
		case '+':
		case '-':
			result = addop;
			break;
		case '*':
		case '/':
		case '%':
			result = multop;
			break;
		case '^':
			result = expop;
			break;
		case '(':
			result = lparen;
			break;
		case ')':
			result = rparen;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			result = digit;
			break;
		case '.':
			result = decimal;
			break;
		case ' ':
			result = space;
			break;
		default:
			result = invalid;
			break;
	}
	return result;
}

Symbol tokenType(token tk)
{
	return type(*tk);
}

int tokenize(char *str, char *(**tokensRef))
{
	char** tokens = NULL;
	char* ptr = str;
	char ch = '\0';
	int numTokens = 0;
	while(ch = *ptr++)
	{
		if(type(ch) == invalid) // Stop tokenizing when we encounter an invalid character
			break;

		char* token = NULL;
		switch(type(ch))
		{
			case addop:
			case multop:
			case lparen:
			case rparen:
				// Assemble a single-character (plus null-terminator) operation token
				{
					token = (char*)malloc(2 * sizeof(char)); // Leave room for '\0'
					token[0] = ch;
					token[1] = '\0';
				}
				break;
			case digit:
			case decimal:
				// Assemble an n-character (plus null-terminator) number token
				{
					int len = 1;
					bool hasDecimal = false;
					// Can only have one decimal point
					if(type(ch) == decimal)
						hasDecimal = true;
					token = (char*)malloc((len + 1) * sizeof(char)); // Leave room for '\0'
					token[0] = ch;
					for(len = 1;
						*ptr // There is a next character and it is not null
						&& (type(*ptr) == digit // The next character is a digit
						 	|| ((type(*ptr) == decimal // Or the next character is a decimal
						 		&& hasDecimal == 0))); // But we have not added a decimal yet
						++len)
					{
						if(type(*ptr) == decimal)
							hasDecimal = true;
						token = (char*)realloc(token, (len + 1) * sizeof(char)); // Leave room for '\0'
						token[len] = *ptr++;
					}
					token[len] = '\0';
				}
				break;
		}
		// Add to list of tokens
		if(token != NULL)
		{
			numTokens++;
			/*if(tokens == NULL) // First allocation
				tokens = (char**)malloc(numTokens * sizeof(char*));
			else*/
				tokens = (char**)realloc(tokens, numTokens * sizeof(char*));
			tokens[numTokens - 1] = token;
		}
	}
	*tokensRef = tokens; // Send back out
	return numTokens;
}

bool leftAssoc(token op)
{
	bool ret;
	switch(tokenType(op))
	{
		case addop:
		case multop:
			ret = true;
			break;
		case expop:
			ret = false;
			break;
	}
	return ret;
}

int precedence(token op1, token op2)
{
	int ret;

	if(tokenType(op1) == tokenType(op2)) // Equal precedence
		ret = 0;
	else if(tokenType(op1) == addop
			&& (tokenType(op2) == multop || tokenType(op2) == expop)) // op1 has lower precedence
		ret = -1;
	else if(tokenType(op2) == addop
			&& (tokenType(op1) == multop || tokenType(op1) == expop)) // op1 has higher precedence
		ret = 1;
	else if(tokenType(op1) == multop
			&& tokenType(op2) == expop) // op1 has lower precedence
		ret = -1;
	else if(tokenType(op1) == expop
			&& tokenType(op2) == multop) // op1 has higher precedence
		ret = 1;

	return ret;
}

void evalStackPush(Stack *s, token val)
{
	switch(tokenType(val))
	{
		case expop:
		case multop:
		case addop:
			{
				if(stackSize(s) >= 2)
				{
					// Pop two operands
					token l, r, res;
					r = (token)stackPop(s);
					l = (token)stackPop(s);

					// Evaluate
					res = doOp(l, val, r);

					// Push result
					stackPush(s, res);
				}
				else
				{
					stackPush(s, val);
				}
			}
			break;
		case digit:
			{
				stackPush(s, val);
			}
			break;
	}
}

bool postfix(token *tokens, int numTokens, Stack *output)
{
	Stack operators;
	int i;
	bool err = false;
	stackInit(&operators);
	for(i = 0; i < numTokens; i++)
	{
		// From Wikipedia/Shunting-yard_algorithm:
		switch(tokenType(tokens[i]))
		{
			case digit:
				{
					// If the token is a number, then add it to the output queue.
					//printf("Adding number to output stack\n");
					evalStackPush(output, tokens[i]);
				}
				break;
			case addop:
			case multop:
			case expop:
				{
					/*
					 * If the token is an operator, op1, then:
					 *     while there is an operator token, op2, at the top of the stack, and
					 *             either op1 is left-associative and its precedence is less than or equal to that of op2,
					 *             or op1 is right-associative and its precedence is less than that of op2,
					 *         pop op2 off the stack, onto the output queue
					 *     push op1 onto the stack
					 */
					while(stackSize(&operators) > 0
						&& (tokenType((char*)stackTop(&operators)) == addop || tokenType((char*)stackTop(&operators)) == multop || tokenType((char*)stackTop(&operators)) == expop)
						&& ((leftAssoc(tokens[i]) && precedence(tokens[i], (char*)stackTop(&operators)) <= 0)
							|| (!leftAssoc(tokens[i]) && precedence(tokens[i], (char*)stackTop(&operators)) < 0)))
					{
						//printf("Moving operator from operator stack to output stack\n");
						evalStackPush(output, stackPop(&operators));
					}
					//printf("Adding operator to operator stack\n");
					stackPush(&operators, tokens[i]);
				}
				break;
			case lparen:
				{
					// If the token is a left paren, then push it onto the stack
					//printf("Adding left paren to operator stack\n");
					stackPush(&operators, tokens[i]);
				}
				break;
			case rparen:
				{
					/*
					 * If the token is a right paren:
					 *     Until the token at the top of the stack is a left paren, pop operators off the stack onto the output queue
					 *     Pop the left paren from the stack, but not onto the output queue
					 *     If the stack runs out without finding a left paren, then there are mismatched parens
					 */
					while(stackSize(&operators) > 0
						&& tokenType((token)stackTop(&operators)) != lparen
						&& stackSize(&operators) > 1)
					{
						//printf("Moving operator from operator stack to output stack\n");
						evalStackPush(output, stackPop(&operators));
					}
					if(stackSize(&operators) > 0
						&& tokenType((token)stackTop(&operators)) != lparen)
					{
						err = true;
						raise(parenMismatch);
					}
					//printf("Removing left paren from operator stack\n");
					stackPop(&operators); // Discard lparen
				}
		}
	}
	/*
	 * When there are no more tokens to read:
	 *     While there are still operator tokens on the stack:
	 *         If the operator token on the top of the stack is a paren, then there are mismatched parens
	 *         Pop the operator onto the output queue
	 */
	while(stackSize(&operators) > 0)
	{
		if(tokenType((token)stackTop(&operators)) == lparen)
		{
			raise(parenMismatch);
			err = true;
		}
		//printf("Moving operator from operator stack to output stack\n");
		evalStackPush(output, stackPop(&operators));
	}
	//stackFree(&operators);
	return err;
}

char* substr(char *str, size_t begin, size_t len)
{
	if(str == NULL
		|| strlen(str) == 0
		|| strlen(str) < (begin+len))
		return NULL;

	char *result = (char*)malloc((len + 1) * sizeof(char));
	int i;
	for(i = 0; i < len; i++)
		result[i] = str[begin+i];
	result[i] = '\0';
	return result;
}

bool strBeginsWith(char *haystack, char *needle)
{
	bool result;
	if(strlen(haystack) < strlen(needle))
	{
		return false;
	}
	else
	{
		char *sub = substr(haystack, 0, strlen(needle));
		result = (strcmp(sub, needle) == 0);
		free(sub);
		sub = NULL;
	}
	return result;
}

int strSplit(char *str, const char split, char *(**partsRef))
{
	char **parts = NULL;
	char *ptr = str;
	char *part = NULL;
	int numParts = 0;
	char ch;
	int len = 0;
	while(1)
	{
		ch = *ptr++;

		if((ch == '\0' || ch == split) && part != NULL) // End of part
		{
			// Add null terminator
			part = (char*)realloc(part, (len+1) * sizeof(char));
			part[len] = '\0';

			// Add to parts
			numParts++;
			if(parts == NULL)
				parts = (char**)malloc(sizeof(char**));
			else
				parts = (char**)realloc(parts, numParts * sizeof(char*));
			parts[numParts - 1] = part;
			part = NULL;
			len = 0;
		}
		else // Add to part
		{
			len++;
			if(part == NULL)
				part = (char*)malloc(sizeof(char));
			else
				part = (char*)realloc(part, len * sizeof(char));
			part[len - 1] = ch;
		}

		if(ch == '\0')
			break;
	}
	*partsRef = parts;
	return numParts;
}

bool isCommand(char *str)
{
	bool result = false;
	char **subs = NULL;
	int len = strSplit(str, ' ', &subs);
	if(len >= 1 && strcmp(subs[0], "set") == 0)
	{
		result = true;
	}
	return result;
}

void execCommand(char *str)
{
	printf("\tcommand \"%s\" recognized\n", str);
}

int main()
{
	char* str = NULL;
	token* tokens = NULL;
	int numTokens = 0;
	Stack expr;
	int i;
	str = ufgets(stdin);
	while(str != NULL && strcmp(str, "quit") != 0)
	{
		if(type(*str) == invalid && isCommand(str))
		{
			// Do something with command
			execCommand(str);

			free(str);
			str = NULL;
		}
		else
		{
			numTokens = tokenize(str, &tokens);
			free(str);
			str = NULL;

			/*printf("\t%d tokens:\n", numTokens);
			for(i = 0; i < numTokens; i++)
			{
				printf("\t\"%s\"", tokens[i]);
				if(type(*tokens[i]) == digit)
					printf(" = %f", buildNumber(tokens[i]));
				printf("\n");
			}*/

			// Convert to postfix
			stackInit(&expr);
			postfix(tokens, numTokens, &expr);
			//stackReverse(&expr);
			/*printf("\tReversed postfix stack:\n\t");
			for(i = 0; i < stackSize(&expr); i++)
			{
				printf("%s ", (token)(expr.content[i]));
			}
			printf("\n");*/
			if(stackSize(&expr) != 1)
			{
				printf("\tError evaluating expression\n");
			}
			else
			{
				token result = stackPop(&expr);
				printf("\t= %s\n", result);
			}

			for(i = 0; i < numTokens; i++)
			{
				free(tokens[i]);
			}
			free(tokens);
			tokens = NULL;
			numTokens = 0;
			stackFree(&expr);
		}

		str = ufgets(stdin);
	}
	free(str);
	str = NULL;
}