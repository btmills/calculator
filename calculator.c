#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // Temporary
#include <getopt.h>
#ifdef CALCULATOR_AS_LIB
#include <errno.h>
#endif
#include "stack.h"
#include "calculator.h"




typedef enum
{
	addop,
	multop,
	expop,
	lparen,
	rparen,
	digit,
	value,
	decimal,
	space,
	text,
	function,
	identifier,
	argsep,
	invalid
} Symbol;

struct Preferences
{
	struct Display
	{
		bool tokens;
		bool postfix;
	} display;
	struct Mode
	{
		bool degrees;
	} mode;
	int precision;
	int maxtokenlength;
} prefs;

typedef enum
{
	divZero,
	overflow,
	parenMismatch,
	inputMissing,
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
		case inputMissing:
			msg = "Function input missing";
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
	/*while(*str && *str != '.')
	{
		result = result * 10 + toDigit(*str++);
	}*/
	result = strtod(str, NULL);
	return result;
}

token num2Str(number num)
{
	int len = 0;
	int precision = MAXPRECISION;
	if (prefs.precision >= 0 && prefs.precision < precision)
		precision = prefs.precision;
	token str = (token)malloc(prefs.maxtokenlength*sizeof(char));
	len = snprintf(str, prefs.maxtokenlength-1, "%.*f", precision, num);
	if (prefs.precision == AUTOPRECISION)
	{
		while (str[len-1] == '0')
		{
			len = snprintf(str, prefs.maxtokenlength-1, "%.*f", --precision, num);
		}
	}

	return str;
}

number toRadians(number degrees)
{
	return degrees * PI / 180.0;
}

number toDegrees(number radians)
{
	return radians * 180.0 / PI;
}

int doFunc(Stack *s, token function)
{
	if (stackSize(s) == 0)
	{
		raise(inputMissing);
		stackPush(s, num2Str(NAN));
		return -1;
	}
	else if (stackSize(s) == 1 && strcmp(stackTop(s), FUNCTIONSEPARATOR) == 0)
	{
		stackPop(s);
		raise(inputMissing);
		stackPush(s, num2Str(NAN));
		return -1;
	}
	token input = (token)stackPop(s);
	number num = buildNumber(input);
	number result = num;
	number counter = 0;

	if(strncmp(function, "abs", 3) == 0)
		result = fabs(num);
	else if(strncmp(function, "floor", 5) == 0)
		result = floor(num);
	else if(strncmp(function, "ceil", 4) == 0)
		result = ceil(num);
	else if(strncmp(function, "sin", 3) == 0)
		result = !prefs.mode.degrees ? sin(num) : sin(toRadians(num));
	else if(strncmp(function, "cos", 3) == 0)
		result = !prefs.mode.degrees ? cos(num) : cos(toRadians(num));
	else if(strncmp(function, "tan", 3) == 0)
		result = !prefs.mode.degrees ? tan(num) : tan(toRadians(num));
	else if(strncmp(function, "arcsin", 6) == 0
		 || strncmp(function, "asin", 4) == 0)
		result = !prefs.mode.degrees ? asin(num) : toDegrees(asin(num));
	else if(strncmp(function, "arccos", 6) == 0
		 || strncmp(function, "acos", 4) == 0)
		result = !prefs.mode.degrees ? acos(num) : toDegrees(acos(num));
	else if(strncmp(function, "arctan", 6) == 0
		 || strncmp(function, "atan", 4) == 0)
		result = !prefs.mode.degrees ? atan(num) : toDegrees(atan(num));
	else if(strncmp(function, "sqrt", 4) == 0)
		result = sqrt(num);
	else if(strncmp(function, "cbrt", 4) == 0)
		result = cbrt(num);
	else if(strncmp(function, "log", 3) == 0)
		result = log(num);
	else if(strncmp(function, "exp", 3) == 0)
		result = exp(num);
	else if(strncmp(function, "min", 3) == 0)
	{
		while (stackSize(s) > 0 && strcmp(stackTop(s), FUNCTIONSEPARATOR) != 0)
		{
			input = (token)stackPop(s);
			num = buildNumber(input);
			if (num < result)
				result = num;
		}
	}
	else if(strncmp(function, "max", 3) == 0)
	{
		while (stackSize(s) > 0 && strcmp(stackTop(s), FUNCTIONSEPARATOR) != 0)
		{
			input = (token)stackPop(s);
			num = buildNumber(input);
			if (num > result)
				result = num;
		}
	}
	else if(strncmp(function, "sum", 3) == 0)
	{
		while (stackSize(s) > 0  && strcmp(stackTop(s), FUNCTIONSEPARATOR) != 0)
		{
			input = (token)stackPop(s);
			num = buildNumber(input);
			result += num;
		}
	}
	else if(strncmp(function, "avg", 3) == 0 ||
			strncmp(function, "mean", 4) == 0)
	{
		// Result already initialized with first number
		counter = 1;
		while (stackSize(s) > 0  && strcmp(stackTop(s), FUNCTIONSEPARATOR) != 0)
		{
			input = (token)stackPop(s);
			num = buildNumber(input);
			result += num;
			counter++;
		}
		result /= counter;
	}
	else if(strncmp(function, "median", 6) == 0)
	{
		// needed for sorting
		Stack tmp, safe;
		// Result already initialized with first number
		counter = 1;
		stackInit(&tmp, (stackSize(s) > 0 ? stackSize(s) : 1));
		stackInit(&safe, (stackSize(s) > 0 ? stackSize(s) : 1));
		// add first value to the later sorted stack
		stackPush(&tmp, input);
		while (stackSize(s) > 0  && strcmp(stackTop(s), FUNCTIONSEPARATOR) != 0)
		{
			input = (token)stackPop(s);
			num = buildNumber(input);
			// save all numbers larger as the stack value
			while (stackSize(&tmp) > 0 && buildNumber(stackTop(&tmp)) < num)
			{
				stackPush(&safe, stackPop(&tmp));
			}
			// push value on the sorted stack
			stackPush(&tmp, input);
			// push all saved numbers back on the sorted stack
			while (stackSize(&safe) > 0)
			{
				stackPush(&tmp, stackPop(&safe));
			}
			counter++;
		}
		stackFree(&safe);
		// calculate the median index
		counter = (number)(((int)counter+1)/2);
		// pop all numbers until median index
		while (counter > 1)
		{
			stackPop(&tmp);
			counter--;
		}
		result = buildNumber(stackPop(&tmp));
		// pop the remaining sorted stack
		while (stackSize(&tmp) > 0)
		{
			stackPop(&tmp);
		}
		stackFree(&tmp);
	}
	else if(strncmp(function, "var", 3) == 0)
	{
		Stack tmp;
		counter = 1;
		// second stack to store values during calculation of mean
		stackInit(&tmp, (stackSize(s) > 0 ? stackSize(s) : 1));
		// push first value to temporary stack
		stackPush(&tmp, input);
		number mean = result;
		while (stackSize(s) > 0  && strcmp(stackTop(s), FUNCTIONSEPARATOR) != 0)
		{
			input = (token)stackPop(s);
			// push value to temporary stack
			stackPush(&tmp, input);
			num = buildNumber(input);
			mean += num;
			counter++;
		}
		// calculate mean
		mean /= counter;
		result = 0;
		// calculate sum of squared differences
		while (stackSize(&tmp) > 0)
		{
			input = (token)stackPop(&tmp);
			num = buildNumber(input)-mean;
			result += pow(num,2);
		}
		// determine variance
		result /= counter;
		stackFree(&tmp);
	}
	if (strcmp(stackTop(s), FUNCTIONSEPARATOR) == 0)
		stackPop(s);
	stackPush(s, num2Str(result));
	return 0;
}

int doOp(Stack *s, token op)
{
	int err = 0;
	token roperand = (token)stackPop(s);
	token loperand = (token)stackPop(s);
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
				{
					raise(divZero);
					if (lside == 0)
						ret = NAN;
					else
						ret = INFINITY;
					err = -1;
				}
				else
					ret = lside / rside;
			}
			break;
		case '%':
			{
				if(rside == 0)
				{
					raise(divZero);
					if (lside == 0)
						ret = NAN;
					else
						ret = INFINITY;
					err = -1;
				}
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
	stackPush(s, num2Str(ret));
	return err;
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
		case '.':
			result = decimal;
			break;
		case ' ':
			result = space;
			break;
		case ',':
			result = argsep;
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
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
			result = text;
			break;
		default:
			result = invalid;
			break;
	}
	return result;
}

bool isFunction(token tk)
{
	return (strncmp(tk, "abs", 3) == 0
		|| strncmp(tk, "floor", 5) == 0
		|| strncmp(tk, "ceil", 4) == 0
		|| strncmp(tk, "sin", 3) == 0
		|| strncmp(tk, "cos", 3) == 0
		|| strncmp(tk, "tan", 3) == 0
		|| strncmp(tk, "arcsin", 6) == 0
		|| strncmp(tk, "arccos", 6) == 0
		|| strncmp(tk, "arctan", 6) == 0
		|| strncmp(tk, "asin", 4) == 0
		|| strncmp(tk, "acos", 4) == 0
		|| strncmp(tk, "atan", 4) == 0
		|| strncmp(tk, "sqrt", 4) == 0
		|| strncmp(tk, "cbrt", 4) == 0
		|| strncmp(tk, "log", 3) == 0
		|| strncmp(tk, "min", 3) == 0
		|| strncmp(tk, "max", 3) == 0
		|| strncmp(tk, "sum", 3) == 0
		|| strncmp(tk, "avg", 3) == 0
		|| strncmp(tk, "mean", 4) == 0
		|| strncmp(tk, "median", 6) == 0
		|| strncmp(tk, "var", 3) == 0
		|| strncmp(tk, "exp", 3) == 0);
}

bool isSpecialValue(token tk)
{
	return (strncmp(tk, "nan", 3) == 0 || strncmp(tk, "inf", 3) == 0);
}

Symbol tokenType(token tk)
{
	if (!tk)
		return invalid;
	Symbol ret = type(*tk);
	switch(ret)
	{
		case text:
			if(isFunction(tk))
				ret = function;
			else if(isSpecialValue(tk))
				ret = value;
			else
				ret = identifier;
			break;
		case addop:
			if(*tk == '-' && strlen(tk) > 1)
				ret = tokenType(tk+1);
			break;
		case decimal:
		case digit:
			ret = value;
			break;
		default:
			break;
	}
	return ret;
}


int tokenize(char *str, char *(**tokensRef))
{
	int i = 0;
	char** tokens = NULL;
	char** tmp = NULL;
	char* ptr = str;
	char ch = '\0';
	int numTokens = 0;
	char* tmpToken = malloc((prefs.maxtokenlength+1) * sizeof(char));
	if (!tmpToken)
	{
		fprintf(stderr, "Malloc of temporary buffer failed\n");
		return 0;
	}
	while((ch = *ptr++))
	{
		if(type(ch) == invalid) // Stop tokenizing when we encounter an invalid character
			break;

		token newToken = NULL;
		tmpToken[0] = '\0';
		switch(type(ch))
		{
			case addop:
				{
					// Check if this is a negative
					if(ch == '-'
						&& (numTokens == 0
							|| (tokenType(tokens[numTokens-1]) == addop
								|| tokenType(tokens[numTokens-1]) == multop
								|| tokenType(tokens[numTokens-1]) == expop
								|| tokenType(tokens[numTokens-1]) == lparen
								|| tokenType(tokens[numTokens-1]) == argsep)))
					{
						// Assemble an n-character (plus null-terminator) number token
						{
							int len = 1;
							bool hasDecimal = false;
							bool hasExponent = false;

							if(type(ch) == decimal) // Allow numbers to start with decimal
							{
								//printf("Decimal\n");
								hasDecimal = true;
								len++;
								tmpToken[0] = '0';
								tmpToken[1] = '.';
							}
							else // Numbers that do not start with decimal
							{
								tmpToken[len-1] = ch;
							}

							// Assemble rest of number
							for(; // Don't change len
								*ptr // There is a next character and it is not null
								&& len <= prefs.maxtokenlength
								&& (type(*ptr) == digit // The next character is a digit
								 	|| ((type(*ptr) == decimal // Or the next character is a decimal
								 		&& hasDecimal == 0)) // But we have not added a decimal
								 	|| ((*ptr == 'E' || *ptr == 'e') // Or the next character is an exponent
								 		&& hasExponent == false) // But we have not added an exponent yet
								|| ((*ptr == '+' || *ptr == '-') && hasExponent == true)); // Exponent with sign
								++len)
							{
								if(type(*ptr) == decimal)
									hasDecimal = true;
								else if(*ptr == 'E' || *ptr == 'e')
									hasExponent = true;
								tmpToken[len] = *ptr++;
							}

							// Append null-terminator
							tmpToken[len] = '\0';
						}
						break;
					}
					// If it's not part of a number, it's an op - fall through
				}
			case multop:
			case expop:
			case lparen:
			case rparen:
			case argsep:
				// Assemble a single-character (plus null-terminator) operation token
				{
					tmpToken[0] = ch;
					tmpToken[1] = '\0';
				}
				break;
			case digit:
			case decimal:
				// Assemble an n-character (plus null-terminator) number token
				{
					int len = 1;
					bool hasDecimal = false;
					bool hasExponent = false;

					if(type(ch) == decimal) // Allow numbers to start with decimal
					{
						//printf("Decimal\n");
						hasDecimal = true;
						len++;
						tmpToken[0] = '0';
						tmpToken[1] = '.';
					}
					else // Numbers that do not start with decimal
					{
						tmpToken[len-1] = ch;
					}

					// Assemble rest of number
					for(; // Don't change len
						*ptr // There is a next character and it is not null
						&& len <= prefs.maxtokenlength
						&& (type(*ptr) == digit // The next character is a digit
						 	|| ((type(*ptr) == decimal // Or the next character is a decimal
						 		&& hasDecimal == 0)) // But we have not added a decimal
						 	|| ((*ptr == 'E' || *ptr == 'e') // Or the next character is an exponent
						 		&& hasExponent == false) // But we have not added an exponent yet
						 	|| ((*ptr == '+' || *ptr == '-') && hasExponent == true)); // Exponent with sign
						++len)
					{
						if(type(*ptr) == decimal)
							hasDecimal = true;
						else if(*ptr == 'E' || *ptr == 'e')
							hasExponent = true;
						tmpToken[len] = *ptr++;
					}

					// Append null-terminator
					tmpToken[len] = '\0';
				}
				break;
			case text:
				// Assemble an n-character (plus null-terminator) text token
				{
					int len = 1;
					tmpToken[0] = ch;
					for(len = 1; *ptr && type(*ptr) == text && len <= prefs.maxtokenlength; ++len)
					{
						tmpToken[len] = *ptr++;
					}
					tmpToken[len] = '\0';
				}
				break;
			default:
				break;
		}
		// Add to list of tokens
		if(tmpToken[0] != '\0' && strlen(tmpToken) > 0)
		{
			numTokens++;
			/*if(tokens == NULL) // First allocation
				tokens = (char**)malloc(numTokens * sizeof(char*));
			else*/
			
			newToken = malloc((strlen(tmpToken)+1) * sizeof(char));
			if (!newToken)
			{
				numTokens--;
				break;
			}
			strcpy(newToken, tmpToken);
			newToken[strlen(tmpToken)] = '\0';
			tmp = (char**)realloc(tokens, numTokens * sizeof(char*));
			if (tmp == NULL)
			{
				if (tokens != NULL)
				{
					for(i=0;i<numTokens-1;i++)
					{
						if (tokens[i] != NULL)
							free(tokens[i]);
					}
					free(tokens);
				}
				*tokensRef = NULL;
				free(newToken);
				free(tmpToken);
				return 0;
			}
			tokens = tmp;
			tmp = NULL;
			tokens[numTokens - 1] = newToken;
		}
	}
	*tokensRef = tokens; // Send back out
	free(tmpToken);
	tmpToken = NULL;
	return numTokens;
}

bool leftAssoc(token op)
{
	bool ret = false;
	switch(tokenType(op))
	{
		case addop:
		case multop:
		
			ret = true;
			break;
		case function:
		case expop:
			ret = false;
			break;
		default:
			break;
	}
	return ret;
}

int precedence(token op1, token op2)
{
	int ret = 0;

	if (op2 == NULL)
		ret = 1;
	else if(tokenType(op1) == tokenType(op2)) // Equal precedence
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
	else if (tokenType(op1) == function 
			&& (tokenType(op2) == addop || tokenType(op2) == multop || tokenType(op2) == expop || tokenType(op2) == lparen))
		ret = 1;
	else if ((tokenType(op1) == addop || tokenType(op1) == multop || tokenType(op1) == expop)
			&& tokenType(op2) == function)
		ret = -1;
	return ret;
}

void evalStackPush(Stack *s, token val)
{
	if(prefs.display.postfix)
		printf("\t%s\n", val);

	switch(tokenType(val))
	{
		case function:
			{
				//token res;
				//operand = (token)stackPop(s);
				if (doFunc(s, val) < 0)
					return;
				//stackPush(s, res);
			}
			break;
		case expop:
		case multop:
		case addop:
			{
				if(stackSize(s) >= 2)
				{
					// Pop two operands

					// Evaluate
					if (doOp(s, val) < 0)
						return;

					// Push result
					//stackPush(s, res);
				}
				else
				{
					stackPush(s, val);
				}
			}
			break;
		case value:
			{
				stackPush(s, val);
			}
			break;
		default:
			break;
	}
}

bool postfix(token *tokens, int numTokens, Stack *output)
{
	Stack operators, intermediate;
	int i;
	bool err = false;
	stackInit(&operators, numTokens);
	stackInit(&intermediate, numTokens);
	for(i = 0; i < numTokens; i++)
	{
		// From Wikipedia/Shunting-yard_algorithm:
		switch(tokenType(tokens[i]))
		{
			case value:
				{
					// If the token is a number, then add it to the output queue.
					//printf("Adding number %s to output stack\n", tokens[i]);
					evalStackPush(output, tokens[i]);
				}
				break;
			case function:
				{
					while(stackSize(&operators) > 0
						&& (tokenType(tokens[i]) != lparen)
						&& ((precedence(tokens[i], (char*)stackTop(&operators)) <= 0)))
					{
						//printf("Moving operator %s from operator stack to output stack\n", (char*)stackTop(&operators));
						evalStackPush(output, stackPop(&operators));
						stackPush(&intermediate, stackTop(output));
					}

					// If the token is a function token, then push it onto the stack.
					//printf("Adding operator %s to operator stack\n", tokens[i]);
					stackPush(&operators, tokens[i]);
				}
				break;
			case argsep:
				{
					/*
					 * If the token is a function argument separator (e.g., a comma):
					 *	 Until the token at the top of the stack is a left
					 *	 paren, pop operators off the stack onto the output
					 *	 queue. If no left paren encountered, either separator
					 *	 was misplaced or parens mismatched.
					 */
					while(stackSize(&operators) > 0
						&& tokenType((token)stackTop(&operators)) != lparen
						&& stackSize(&operators) > 1)
					{
						//printf("Moving operator from operator stack to output stack\n");
						evalStackPush(output, stackPop(&operators));
						stackPush(&intermediate, stackTop(output));
					}
					/*if(stackSize(&operators) > 0
						&& tokenType((token)stackTop(&operators)) != lparen)
					{
						err = true;
						raise(parenMismatch);
					}
					printf("Removing left paren from operator stack\n");
					stackPop(&operators); // Discard lparen*/
				}
				break;
			case addop:
			case multop:
			case expop:
				{
					/*
					 * If the token is an operator, op1, then:
					 *	 while there is an operator token, op2, at the top of the stack, and
					 *			 either op1 is left-associative and its precedence is less than or equal to that of op2,
					 *			 or op1 is right-associative and its precedence is less than that of op2,
					 *		 pop op2 off the stack, onto the output queue
					 *	 push op1 onto the stack
					 */
					while(stackSize(&operators) > 0
						&& (tokenType((char*)stackTop(&operators)) == addop || tokenType((char*)stackTop(&operators)) == multop || tokenType((char*)stackTop(&operators)) == expop)
						&& ((leftAssoc(tokens[i]) && precedence(tokens[i], (char*)stackTop(&operators)) <= 0)
							|| (!leftAssoc(tokens[i]) && precedence(tokens[i], (char*)stackTop(&operators)) < 0)))
					{
						//printf("Moving operator %s from operator stack to output stack\n", (char*)stackTop(&operators));
						evalStackPush(output, stackPop(&operators));
						stackPush(&intermediate, stackTop(output));
					}
					//printf("Adding operator %s to operator stack\n", tokens[i]);
					stackPush(&operators, tokens[i]);
				}
				break;
			case lparen:
				{
					// If the token is a left paren, then push it onto the stack
					//printf("Adding left paren to operator stack\n");
					if (tokenType(stackTop(&operators)) == function)
						stackPush(output, FUNCTIONSEPARATOR);
					stackPush(&operators, tokens[i]);
				}
				break;
			case rparen:
				{
					/*
					 * If the token is a right paren:
					 *	 Until the token at the top of the stack is a left paren, pop operators off the stack onto the output queue
					 *	 Pop the left paren from the stack, but not onto the output queue
					 *	 If the stack runs out without finding a left paren, then there are mismatched parens
					 */
					while(stackSize(&operators) > 0
						&& tokenType((token)stackTop(&operators)) != lparen
						&& stackSize(&operators) > 1)
					{
						//printf("Moving operator %s from operator stack to output stack\n", (char*)stackTop(&operators));
						evalStackPush(output, stackPop(&operators));
						stackPush(&intermediate, stackTop(output));
					}
					if(stackSize(&operators) > 0
						&& tokenType((token)stackTop(&operators)) != lparen)
					{
						err = true;
						raise(parenMismatch);
					}
					//printf("Removing left paren from operator stack\n");
					stackPop(&operators); // Discard lparen
					while (stackSize(&operators) > 0 && tokenType((token)stackTop(&operators)) == function)
					{
						//printf("Removing function from operator stack to output stack\n");
						evalStackPush(output, stackPop(&operators));
						stackPush(&intermediate, stackTop(output));
					}
				}
				break;
			default:
				break;
		}
	}
	/*
	 * When there are no more tokens to read:
	 *	 While there are still operator tokens on the stack:
	 *		 If the operator token on the top of the stack is a paren, then there are mismatched parens
	 *		 Pop the operator onto the output queue
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
		stackPush(&intermediate, stackTop(output));
	}
	// pop result from intermediate stack
	stackPop(&intermediate);
	// free remaining intermediate results
	while (stackSize(&intermediate) > 0)
	{
		token s = stackPop(&intermediate);
		free(s);
	}
	if (err == true)
	{
		while (stackSize(&operators) > 0)
		{
			token s = stackPop(&operators);
			//printf("Freeing %s from operators stack\n", s);
			free(s);
		}
	}
	stackFree(&intermediate);
	stackFree(&operators);
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
	char **tmpparts = NULL;
	char *ptr = str;
	char *part = NULL;
	char *tmppart = NULL;
	int numParts = 0;
	char ch;
	int len = 0;
	while(1)
	{
		ch = *ptr++;

		if((ch == '\0' || ch == split) && part != NULL) // End of part
		{
			// Add null terminator
			tmppart = (char*)realloc(part, (len+1) * sizeof(char));
			// if realloc fails, free current part and all previous parts
			if (tmppart == NULL)
			{
				free(part);
				part = NULL;
				for(len=0;len<numParts;len++)
				{
					if (parts[len])
						free(parts[len]);
				}
				if (parts)
					free(parts);
				parts = NULL;
				numParts = 0;
				break;
			}
			part = tmppart;
			part[len] = '\0';

			// Add to parts
			numParts++;
			if(parts == NULL)
				parts = (char**)malloc(sizeof(char**));
			else
			{
				tmpparts = (char**)realloc(parts, numParts * sizeof(char*));
				// if relloc fails, free current and previous parts
				if (tmpparts == NULL)
				{
					free(part);
					part = NULL;
					for(len=0;len<numParts-1;len++)
					{
						if (parts[len])
							free(parts[len]);
					}
					if (parts)
						free(parts);
					parts = NULL;
					numParts = 0;
					break;
				}
				parts = tmpparts;
			}
			parts[numParts - 1] = part;
			part = NULL;
			len = 0;
		}
		else // Add to part
		{
			len++;
			if(part == NULL)
			{
				part = (char*)malloc(sizeof(char));
			}
			else
			{
				tmppart = (char*)realloc(part, len * sizeof(char));
				// if relloc fails, free current and previous parts
				if (tmppart == NULL)
				{
					free(part);
					part = NULL;
					for(len=0;len<numParts;len++)
					{
						if (parts[len])
							free(parts[len]);
					}
					free(parts);
					numParts = 0;
					parts = NULL;
					break;
				}
				part = tmppart;
			}
			part[len - 1] = ch;
		}

		if(ch == '\0')
			break;
	}
	*partsRef = parts;
	return numParts;
}

/*bool isCommand(char *str)
{
	bool result = false;
	char **words = NULL;
	int len = strSplit(str, ' ', &words);
	if(len >= 1 && strcmp(words[0], "set") == 0)
	{
		result = true;
	}
	return result;
}*/

bool execCommand(char *str)
{
	int i = 0;
	bool recognized = false;
	char **words = NULL;
	int len = strSplit(str, ' ', &words);
	if(len >= 1 && strcmp(words[0], "get") == 0)
	{
		if(len >= 2 && strcmp(words[1], "display") == 0)
		{
			if(len >= 3 && strcmp(words[2], "tokens") == 0)
			{
				recognized = true;
				printf("\t%s\n", (prefs.display.tokens ? "on" : "off"));
			}
			else if(len >= 3 && strcmp(words[2], "postfix") == 0)
			{
				recognized = true;
				printf("\t%s\n", (prefs.display.postfix ? "on" : "off"));
			}
		}
		else if(len >= 2 && strcmp(words[1], "mode") == 0)
		{
			recognized = true;
			printf("\t%s\n", (prefs.mode.degrees ? "degrees" : "radians"));
		}
		else if(len >= 2 && strcmp(words[1], "precision") == 0)
		{
			recognized = true;
			if (prefs.precision > 0)
				printf("\t%d\n", prefs.precision);
			else
				printf("\tauto\n");
		}
	}
	else if(len >= 1 && strcmp(words[0], "set") == 0)
	{
		if(len >= 2 && strcmp(words[1], "display") == 0)
		{
			if(len >= 3 && strcmp(words[2], "tokens") == 0)
			{
				if(len >= 4 && strcmp(words[3], "on") == 0)
				{
					recognized = true;
					prefs.display.tokens = true;
				}
				else if(len >= 4 && strcmp(words[3], "off") == 0)
				{
					recognized = true;
					prefs.display.tokens = false;
				}
			}
			else if(len >= 3 && strcmp(words[2], "postfix") == 0)
			{
				if(len >= 4 && strcmp(words[3], "on") == 0)
				{
					recognized = true;
					prefs.display.postfix = true;
				}
				else if(len >= 4 && strcmp(words[3], "off") == 0)
				{
					recognized = true;
					prefs.display.postfix = false;
				}
			}
		}
		else if(len >= 2 && strcmp(words[1], "mode") == 0)
		{
			if(len >= 3 && strcmp(words[2], "radians") == 0)
			{
				recognized = true;
				prefs.mode.degrees = false;
			}
			else if(len >= 3 && strcmp(words[2], "degrees") == 0)
			{
				recognized = true;
				prefs.mode.degrees = true;
			}
		}
		else if (len >= 2 && strcmp(words[1], "precision") == 0)
		{
			if(len >= 3 && strcmp(words[2], "auto") == 0)
			{
				recognized = true;
				prefs.precision = -1;
			}
			else if (len >= 3 && type(words[2][0]) == digit)
			{
				recognized = true;
				prefs.precision = atoi(words[2]);
			}
		}
	}
	if (words)
	{
		for (i=0;i<len;i++)
		{
			if (words[i])
				free(words[i]);
		}
		free(words);
	}

	return recognized;
}

#ifdef CALCULATOR_AS_LIB

int calculator_init()
{
	prefs.precision = DEFAULTPRECISION;
	prefs.maxtokenlength = MAXTOKENLENGTH;
	return 0;
}

int calculator_setprecision(int precision)
{
	if ((precision >= 0 && precision < MAXPRECISION) || (precision == AUTOPRECISION))
	{
		prefs.precision = precision;
		return 0;
	}
	return -EINVAL;
}

int calculator_setpostfix(int flag)
{
	if (flag >= 0 && flag <= 1)
	{
		prefs.display.tokens = flag;
		return 0;
	}
	return -EINVAL;
}

int calculator_settokenlength(int tokenlength)
{
	if (tokenlength >= 0 && tokenlength < MAXTOKENLENGTH)
	{
		prefs.maxtokenlength = tokenlength;
		return 0;
	}
	return -EINVAL;
}

int calculator_calc(const char* formula, double* result)
{
	int i = 0;
	int err = 0;
	int numTokens = 0;
	double res = NAN;
	Stack expr;
	token* tokens = NULL;
	if (strlen(formula) == 0)
	{
		return -1;
	}
	if (type(*formula) == text)
	{
		err = execCommand((char*)formula);
		return err;
	}
	numTokens = tokenize((char*)formula, &tokens);
	if (numTokens == 0)
	{
		printf("\tError tokenizing expression\n");
		return -EINVAL;
	}
	stackInit(&expr, numTokens);
	postfix(tokens, numTokens, &expr);
	if(stackSize(&expr) != 1)
	{
		printf("\tError evaluating expression\n");
		return -EFAULT;
	}
	else
	{
		res = atof((char*)stackTop(&expr));
		for (i=0; i< numTokens; i++)
		{
			if (tokens[i] == stackTop(&expr))
				tokens[i] = NULL;
		}
		free(stackPop(&expr));
	}

	for(i = 0; i < numTokens; i++)
	{
		if (tokens[i] != NULL)
			free(tokens[i]);
	}
	free(tokens);
	tokens = NULL;
	numTokens = 0;
	stackFree(&expr);
	if (result)
		*result = res;
	return 0;
}


#else
int main(int argc, char *argv[])
{
	char* str = NULL;
	token* tokens = NULL;
	int numTokens = 0;
	Stack expr;
	int i;
	int ch, rflag = 0;
	prefs.precision = DEFAULTPRECISION;
	prefs.maxtokenlength = MAXTOKENLENGTH;

	while ((ch = getopt(argc, argv, "rm:")) != -1) {
		switch (ch) {
			case 'r':
				rflag = 1;
				break;
			case 'm':
				prefs.maxtokenlength = atoi(optarg);
		}
	}
	str = ufgets(stdin);
	while(str != NULL && strcmp(str, "quit") != 0)
	{
		if (strlen(str) == 0)
			goto get_new_string;
		if(type(*str) == text)
		{
			// Do something with command
			if (!execCommand(str))
				goto no_command;

			free(str);
			str = NULL;
		}
		else
		{
no_command:
			numTokens = tokenize(str, &tokens);
			free(str);
			str = NULL;

			if(prefs.display.tokens)
			{
				printf("\t%d tokens:\n", numTokens);
				for(i = 0; i < numTokens; i++)
				{
					printf("\t\"%s\"", tokens[i]);
					if(tokenType(tokens[i]) == value)
						printf(" = %f", buildNumber(tokens[i]));
					printf("\n");
				}
			}

			// Convert to postfix
			stackInit(&expr, numTokens);
			if(prefs.display.postfix)
				printf("\tPostfix stack:\n");
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
				if (!rflag)
					printf("\t= ");
				printf("%s\n", (char*)stackTop(&expr));
				for (i=0; i< numTokens; i++)
				{
					if (tokens[i] == stackTop(&expr))
						tokens[i] = NULL;
				}
				free(stackPop(&expr));
			}

			for(i = 0; i < numTokens; i++)
			{
				if (tokens[i] != NULL)
					free(tokens[i]);
			}
			free(tokens);
			tokens = NULL;
			numTokens = 0;
			stackFree(&expr);
		}
get_new_string:
		str = ufgets(stdin);
	}

	free(str);
	str = NULL;


	return EXIT_SUCCESS;
}
#endif
