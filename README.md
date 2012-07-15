# Calculator

## What

This is a command-line calculator, written in C, supporting the standard mathematical operations and a set of functions. Commands can be entered in standard infix syntax, with parentheses denoting nonstandard order of operations. You know, like you were taught in elementary and middle school.

### Operations
*	\+ Addition
*	\- Subtraction
*	\* Multiplication
*	/ Division
*	^ Exponent
*	% Modulus

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

## Why

To investigate tokenization of an input string, mathematical expressions in infix and postfix notation, and an untyped stack data structure.

## How

Computation begins with the tokenization of the input string, maintaining original infix order. They are then converted to postfix notation (Reverse Polish Notation) for evaluation using the [shunting-yard algorithm][sy]. As operators are pushed onto the postfix stack, terms are evaluated.

[sy]: http://en.wikipedia.org/wiki/Shunting_yard_algorithm "Wikipedia article on the shunting-yard algorithm"

## What's Cool

1. **Untyped stack**
Elements are stored on the stack as `void *`s. This means that any type of element can be stored on the stack at the same time as any element of any other type - including custom types - as long as an element's type is known when it is popped off the stack.

1. **Early evaluation**
There is no separate evaluation step; elements are evaluated on the postfix stack as soon as all terms are available. Due to the nature of the [shunting-yard algorithm][sy], as soon as an operator is pushed, it can be evaluated. When the `evalStackPush()` function sees an operator, rather than pushing it, pops its operands, runs the computation, and pushes the result. This means that once the input expression has been converted to postfix notation, the only element on the postfix stack is the result of the calculation.
