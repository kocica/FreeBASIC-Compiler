/**
 * @file target_code_gen.c
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

#include "target_code_gen.h"


void INSTR_GEN_INIT(instr_generator *instr_gen)
{
        instr_gen->First = NULL;
        instr_gen->Last =NULL;
}

void INSTR_GEN_FREE(instr_generator *instr_gen)
{
        instr * tmp;

        while (instr_gen->First != NULL)
        {
                tmp = instr_gen->First;
                instr_gen->First = instr_gen->First->rptr;
                free(tmp->str);
                free(tmp);
        }

        instr_gen->First = instr_gen->Last = NULL;
}

void INSTR_PRINT(instr_generator *instr_gen)
{
        instr * tmp = instr_gen->First;

        while (tmp != NULL)
        {
                printf("%s\n", tmp->str);
                tmp = tmp->rptr;
        }
}

void INSTR_GEN(instr_generator *instr_gen, const char *instr_string)
{
        instr * i = malloc(sizeof *i);

        if (i == NULL)
        {
                // E_INTERNAL
                return;
        }

        i->str = malloc(strlen(instr_string) + 1);

        if (i->str == NULL)
        {
                // E_INTERNAL
                return;
        }

        strcpy(i->str, instr_string);

        if (instr_gen->First == NULL)
        {
                instr_gen->Last = i;
                instr_gen->Last->lptr = NULL;
                instr_gen->Last->rptr = NULL;
                instr_gen->First = instr_gen->Last;
        }
        else
        {
                i->rptr = NULL;
                i->lptr = instr_gen->Last;
                instr_gen->Last->rptr = i;
                instr_gen->Last = i;
        }
}

void INSTR_GEN_ADD_S(instr_generator *instr_gen, const char *instr_string)
{
        instr * i = instr_gen->Last;

        char * tmp_s = malloc(strlen(i->str) + 1);
        strcpy(tmp_s, i->str);

        free(i->str);

        i->str = malloc(strlen(tmp_s) + strlen(instr_string) + 1);
        strcpy(i->str, tmp_s);
        free(tmp_s);
        strcat(i->str, instr_string);
}

void INSTR_GEN_ADD_I(instr_generator *instr_gen, const int instr_int)
{
        instr * i = instr_gen->Last;

        char * tmp_int = malloc(100);
        sprintf(tmp_int, "%d", instr_int);


        char * tmp_s = malloc(strlen(i->str) + 1);
        strcpy(tmp_s, i->str);

        free(i->str);

        i->str = malloc(strlen(tmp_s) + strlen(tmp_int) + 1);
        strcpy(i->str, tmp_s);
        strcat(i->str, tmp_int);

        free(tmp_s);
        free(tmp_int);
}

void INSTR_GEN_ADD_F(instr_generator *instr_gen, const float instr_float)
{
        instr * i = instr_gen->Last;

        char * tmp_float = malloc(100);
        sprintf(tmp_float, "%g", instr_float);


        char * tmp_s = malloc(strlen(i->str) + 1);
        strcpy(tmp_s, i->str);

        free(i->str);

        i->str = malloc(strlen(tmp_s) + strlen(tmp_float) + 1);
        strcpy(i->str, tmp_s);
        strcat(i->str, tmp_float);

        free(tmp_s);
        free(tmp_float);
}