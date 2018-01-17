/**
 * @file string.c
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

#include "string.h"

/** @brief Resizing is expensive so we reallocate by these chunks */
static int STRING_RESIZE_CHUNK = 32;

void string_init(String *s)
{
        s->pString = malloc(STRING_RESIZE_CHUNK);
        for (int i = 0; i < STRING_RESIZE_CHUNK; i++)
        {
                s->pString[i] = '\0';
        }
        s->act_size = 0;
        s->alloc_size = STRING_RESIZE_CHUNK;
}

void string_free(String *s)
{
        free(s->pString);
        s->pString = NULL;
        s->act_size = 0;
        s->alloc_size = 0;
}

String string_copy_ctor(String *from)
{
        String to;

        to.pString = malloc(from->act_size);
        to.alloc_size = from->act_size;
        to.act_size = from->act_size;

        for (size_t i = 0; i < from->act_size; i++)
        {
                to.pString[i] = from->pString[i];
        }

        return to;
}

String string_move_ctor(String *from)
{
        String to;

        to.pString = from->pString;
        to.alloc_size = from->alloc_size;
        to.act_size = from->act_size;

        from->pString = NULL;
        from->alloc_size = 0;
        from->act_size = 0;

        return to;
}

void string_push_char(String *s, char c)
{
        if (s->act_size >= s->alloc_size)
        {
                char *toFree = s->pString;
                s->pString = realloc(s->pString, s->alloc_size + STRING_RESIZE_CHUNK);
                if (s->pString == NULL)
                {
                        s->pString = toFree;
                        string_free(s);
                        //E_INTERN;
                }
                s->alloc_size += STRING_RESIZE_CHUNK;
        }

        if (s->act_size != 0)
        {
                s->act_size--;
        }

        s->pString[s->act_size] = c;
        s->pString[s->act_size+1] = '\0';
        s->act_size+=2;
}

void string_pop_char(String *s)
{
        if (s->act_size > 1)
        {
                s->act_size--;
                s->pString[s->act_size-1] = '\0';
        }
        else
        {
                s->act_size = 0;
                s->pString[0] = '\0';
        }
}

void string_append_string(String *s1, char *s2)
{
        size_t s2size = strlen(s2);

        if ((s1->alloc_size - s1->act_size) < s2size)
        {
                char *toFree = s1->pString;
                s1->pString = realloc(s1->pString, s1->alloc_size + s2size + 1);
                if (s1->pString == NULL)
                {
                        s1->pString = toFree;
                        string_free(s1);
                        //E_INTERN;
                }
                s1->alloc_size += s2size + 1;
        }

        if (s1->act_size != 0)
        {
                s1->act_size--;
        }

        for(size_t i = 0; i < s2size; i++)
        {
                s1->pString[s1->act_size] = s2[i];
                s1->act_size++;
        }

        s1->pString[s1->act_size] = '\0';
        s1->act_size++;
}

bool string_compare(const String *s1, const String *s2)
{
        if (s1->act_size != s2->act_size)
        {
                return false;
        }

        for (size_t i = 0; i < s1->act_size; i++)
        {
                if (s1->pString[i] != s2->pString[i])
                {
                        return false;
                }
        }

        return true;
}

void string_to_upper(String *s)
{
        for (size_t i = 0; i < s->act_size; i++)
        {
                if (s->pString[i] >= 'a' && s->pString[i] <= 'z')
                {
                        s->pString[i] += ('A' - 'a');
                }
        }
}

void string_to_lower(String *s)
{
        for (size_t i = 0; i < s->act_size; i++)
        {
                if (s->pString[i] >= 'A' && s->pString[i] <= 'Z')
                {
                        s->pString[i] -= ('A' - 'a');
                }
        }
}

bool string_empty(const String *s)
{
        return (s->act_size == 0);
}

size_t string_get_size(const String *s)
{
        return (s->act_size == 0 ? 0 : s->act_size - 1);
}

char * string_get_string(const String *s)
{
        return s->pString;
}

void string_clear(String *s)
{
        for (size_t i = 0; i < s->act_size; i++)
        {
                s->pString[i] = '\0';
        }
        s->act_size = 0;
}

void string_terminate(String *s)
{
        s->pString[0] = 0;
        s->act_size = 0;
}
