/**
 * @file stack.c
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

#include "stack.h"

static const int STACK_RESIZE_CHUNK = 10;
static const int STACK_EMPTY = -1;

void stack_init (TStack* s)
{
        s->arr = malloc ((sizeof *(s->arr)) * STACK_RESIZE_CHUNK);
        s->top = STACK_EMPTY;
        s->allocated = STACK_RESIZE_CHUNK;
}

void stack_init_string (TStackString* s)
{
        s->arr = malloc ((sizeof *(s->arr)) * STACK_RESIZE_CHUNK);
        s->top = STACK_EMPTY;
        s->allocated = STACK_RESIZE_CHUNK;
}


void stack_free (TStack* s)
{
        s->top = STACK_EMPTY;
        s->allocated = 0;
        free(s->arr);
        s->arr = NULL;
}

void stack_free_string (TStackString* s)
{
        for (int i = 0; i < s->top; i++)
        {
                free(s->arr[i]);
        }
        s->top = STACK_EMPTY;
        s->allocated = 0;
        free(s->arr);
        s->arr = NULL;
}

void stack_clear(TStack *s)
{
        s->top = STACK_EMPTY;
        s->arr = NULL;
        s->allocated = 0;
}

bool stack_empty (const TStack* s)
{
        return (s->top == STACK_EMPTY);
}

size_t stack_size (const TStack* s)
{
        return (s->top + 1);
}

void stack_pop (TStack* s)
{
        if (s->top != STACK_EMPTY)
        {
                // Decrease pointer to top of stack
                s->top--;
        }
}

int stack_top(const TStack* s)
{
        return s->arr[s->top];
}

void stack_push(TStack* s, int i)
{
        s->top++;

        if (s->top >= s->allocated)
        {
                int *toFree = s->arr;
                s->arr = realloc(s->arr, (sizeof *(s->arr)) * (s->allocated + STACK_RESIZE_CHUNK));
                s->allocated += STACK_RESIZE_CHUNK;
                if (s->arr == NULL)
                {
                        // ERROR E_INTERN; Failed to allocate memory
                        free(toFree);
                }
        }

        s->arr[s->top] = i;
}

void stack_push_string(TStackString* s, char *str)
{
        s->top++;

        if (s->top >= s->allocated)
        {
                char *toFree = *s->arr;
                s->arr = realloc(s->arr, (sizeof *(s->arr)) * (s->allocated + STACK_RESIZE_CHUNK));
                s->allocated += STACK_RESIZE_CHUNK;
                if (s->arr == NULL)
                {
                        // ERROR E_INTERN; Failed to allocate memory
                        free(toFree);
                }
        }

        s->arr[s->top] = str;
}
