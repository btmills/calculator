# Calculator

## What

This is a command-line calculator, written in C, supporting the standard mathematical operations and a set of functions. Commands can be entered in standard infix syntax, with parentheses denoting nonstandard order of operations. You know, like you were taught in elementary and middle school.

### Operators
*	`+` Addition
*	`-` Subtraction
*	`*` Multiplication
*	`/` Division
*	`^` Exponent
*	`%` Modulus

### Functions
*	`abs(...)` Absolute value
*	`floor(...)` Floor
*	`ceil(...)` Ceiling
*	`sin(...)` Sine
*	`cos(...)` Cosine
*	`tan(...)` Tangent
*	`arcsin(...)` Arcsine
*	`arccos(...)` Arccosine
*	`arctan(...)` Arctangent
*	`sqrt(...)` Square root
*	`cbrt(...)` Cube root
*	`log(...)` Logarithm
*	`exp(...)` Exponentiation (e^x)
*	`min(...)` Minimum
*	`max(...)` Maximum
*	`sum(...)` Summation
*	`mean(...)` Mean
*	`avg(...)` Mean
*	`median(...)` Median
*	`var(...)` Variance

## Why

To investigate tokenization of an input string, mathematical expressions in infix and postfix notation, and an untyped stack data structure.

## How

Computation begins with the tokenization of the input string, maintaining original infix order. They are then converted to postfix notation (Reverse Polish Notation) for evaluation using the [shunting-yard algorithm][sy]. As operators are pushed onto the postfix stack, terms are evaluated.

[sy]: http://en.wikipedia.org/wiki/Shunting_yard_algorithm "Wikipedia article on the shunting-yard algorithm"

## What's Cool

1. **Untyped stack**  
Elements are stored on the stack as `void *` types. This means that any type of element can be stored on the stack at the same time as any element of any other type - including custom types - as long as an element's type is known when it is popped off the stack.

1. **Early evaluation**  
There is no separate evaluation step; elements are evaluated on the postfix stack as soon as all terms are available. Due to the nature of the [shunting-yard algorithm][sy], as soon as an operator is pushed, it can be evaluated. When the `evalStackPush()` function sees an operator, rather than pushing it, pops its operands, runs the computation, and pushes the result. This means that once the input expression has been converted to postfix notation, the only element on the postfix stack is the result of the calculation.

1. **Settings**  
Try typing `set display tokens on` and entering an expression. The calculator will display the result of its tokenization. Starting the calculator and entering `get mode` reveals that it defaults to `radians`. Options are as follows:
	* `get/set` Read or change the value of a setting
	* `display` Settings to do with the display of the evaluation process
		* `postfix (off/on)` Display the postfix stack before evaluation
		* `tokens (off/on)` Display the result of tokenization
	* `mode (radians/degrees)` Evaluation mode of trigonometric functions
	* `precision (X/auto)` Set precision to `X` decimal places or `auto` to reduce decimal places as far as possible without loosing precision. Default is 5 decimal places

## Building and Running

Build with `make`. Clean with `make clean`. Run with `./calc`. Type any mathematical expression, for example, `3*(2^4) - 3*floor(2 * sin(3.14 / 2))` and press the Enter key. Type `quit` to close.

There is a `-r` command line option which removes the `=` from the output, outputting only the result value. This is designed for use in situations such as shell scripting, where only the raw, unprocessed value is desired.

There is a `-m` command line option which sets the maximal length of a token. Default is 512 characters.
