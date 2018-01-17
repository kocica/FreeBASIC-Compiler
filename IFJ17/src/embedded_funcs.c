/**
 * @file embedded_funcs.c
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

#include "embedded_funcs.h"

int Length(char *s)
{
        int len = 0;
        while (*s++)
        {
                len++;
        }
        return len;
}

char * SubStr(char *s, int i, int n)
{
        int str_len = Length(s);
        char * substr;

        if (s == NULL || i <= 0)
        {
                return NULL;
        }
        else if (n < 0 || (n > (str_len - i)))
        {
                i--;
                substr = malloc(str_len - i + 1);
                if (substr == NULL)
                {
                        // E
                }
                strncpy(substr, s + i, str_len - i);
                substr[str_len - i] = '\0';
                return substr;
        }
        i--;
        substr = malloc(n + 1);
        if (substr == NULL)
        {
                // E
        }
        strncpy(substr, s + i, n);
        substr[n] = '\0';
        return substr;
}

int Asc(char *s, int i)
{
        return ((i < 0 || i >= Length(s)) ? 0 : (int)(*(s+i)));
}

char * Chr(int i)
{
        char *ret = malloc(2);
        ret[0] = (char)i;
        ret[1] = '\0';

        return ret;
}

void add_funcs_to_htab(htab * ht)
{
        function f = { .defined = true, .embedded = true, .declared = false };
        TStack params;


        /**
              int Length(char *s)
         */
        stack_init(&params);
        stack_push(&params, VT_STRING);
        f.return_type = VT_INT;
        f.params.top = params.top;
        f.params.allocated = params.allocated;
        f.params.arr = params.arr;
        htab_add_func(ht, &f, "LENGTH");


        /**
              int Asc(char *s, int i)
         */
        stack_init(&params);
        stack_push(&params, VT_STRING);
        stack_push(&params, VT_INT);
        f.return_type = VT_INT;
        f.params.top = params.top;
        f.params.allocated = params.allocated;
        f.params.arr = params.arr;
        htab_add_func(ht, &f, "ASC");

        /**
              char * Chr(int i)
         */
        stack_init(&params);
        stack_push(&params, VT_INT);
        f.return_type = VT_STRING;
        f.params.top = params.top;
        f.params.allocated = params.allocated;
        f.params.arr = params.arr;
        htab_add_func(ht, &f, "CHR");

        /**
              char * SubStr(char *s, int i, int n)
         */
        stack_init(&params);
        stack_push(&params, VT_STRING);
        stack_push(&params, VT_INT);
        stack_push(&params, VT_INT);
        f.return_type = VT_STRING;
        f.params.top = params.top;
        f.params.allocated = params.allocated;
        f.params.arr = params.arr;
        htab_add_func(ht, &f, "SUBSTR");
}