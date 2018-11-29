/**
 * @file stack.h
 *
 * IAL; IFJ project 2017 -- IFJ17 Compiler
 *
 * <br><b>IFJ17 Compiler<b>
 * <br>
 * <br>Compiles IFJ17 language (subset of FreeBASIC) source code to  IFJcode17 intercode.
 * <br>
 * <br> 1) Read source code from standard input
 * <br> 2) Perform syntactic analyzation
 * <br> 3) Compile source code to target code
 * <br> 4) Print intercode to standard output
 * <br>
 * <br><b>Team<b>
 * <br>  Filip Kocica   (xkocic01)
 * <br>  Matej Knazik   (xknazi00)
 * <br>  Andrea Fickova (xficko00)
 * <br>  Jiri Fiala     (xfiala54)
 *
 * @author(s) Filip Kocica (xkocic01@fit.vutbr.cz), FIT
 * @date      24.9.2017
 */

#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "token.h"

/** Stack of integers, used for storing function params
      to check them in semantic analyzation */
typedef struct
{
        int top;
        int allocated;
        int *arr;
} TStack;

/** Stack of strings, used for storing function param names
      to use them in intercode */
typedef struct
{
        int top;
        int allocated;
        char **arr;
} TStackString;


/** Inits stack */
void stack_init(TStack* s);
/** Frees stack */
void stack_free(TStack* s);
/** Doesnt free the stack just clears the content */
void stack_clear(TStack *s);

// Get functions
/** Returns true if stack is empty else false */
bool stack_empty(const TStack* s);
/** Returns size of stack */
size_t stack_size(const TStack* s);
/** Returns top most item of stack */
int stack_top(const TStack* s);

// Push / pop
/** Pop the top most item in stack */
void stack_pop(TStack* s);
/** Push the next param to stack */
void stack_push(TStack* s, int i);

/** Inits stack */
void stack_init_string(TStackString* s);
/** Frees stack */
void stack_free_string(TStackString* s);
/** Push the next param name to stack */
void stack_push_string(TStackString* s, char *str);

#endif /* ! STACK_H */
