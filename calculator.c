#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // Temporary

#define bool char
#define true 1
#define false 0

typedef enum
{
	addop,
	multop,
	expop,
	paren,
	digit,
	decimal,
	space,
	invalid
} Symbol;

typedef enum
{
	divZero,
	overflow
} error;

typedef char* token;

typedef double number;

void raise(error err)
{
	char* msg;
	switch(err)
	{
		case divZero:
			msg = "Error: Divide by zero";
			break;
		case overflow:
			msg = "Error: Overflow";
			break;
	}
}

number doAdd(number lside, token op, number rside)
{
	number result;
	switch(*op)
	{
		case '+':
			{
				result = lside + rside;
			}
			break;
		case '-':
			{
				result = lside - rside;
			}
			break;
	}
	return result;
}

number doMult(number lside, token op, number rside)
{
	number result;
	switch(*op)
	{
		case '*':
			{
				result = lside * rside;
			}
			break;
		case '/':
			{
				if(rside == 0)
					raise(divZero);
				else
					result = lside * rside;
			}
			break;
		case '%':
			{
				if(rside == 0)
					raise(divZero);
				else
				{
					result = (int)(lside / rside);
					result = lside - ((number)result * rside);
				}
			}
	}
	return result;
}

number doPow(number lside, number rside)
{
	number result;
	result = pow(lside, rside);
	return result;
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
		case ')':
			result = paren;
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

unsigned int toDigit(char ch)
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
			case paren:
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

			printf("\t%d tokens:\n", numTokens);
			for(i = 0; i < numTokens; i++)
			{
				printf("\t\"%s\"", tokens[i]);
				if(type(*tokens[i]) == digit)
					printf(" = %f", buildNumber(tokens[i]));
				printf("\n");
			}

			for(i = 0; i < numTokens; i++)
			{
				free(tokens[i]);
			}
			free(tokens);
			tokens = NULL;
			numTokens = 0;
		}

		str = ufgets(stdin);
	}
	free(str);
	str = NULL;
}