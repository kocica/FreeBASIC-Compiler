/**
 * @file symtable.c
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

#include "symtable.h"

/** @brief Initial size of hash table */
static const int HTAB_SIZE = 500000;

/**
 * @brief Hashes string to number
 *
 * @param str [in] String to hash
 *
 * @return hash number
 */
static unsigned int hash_function(const char * str);

size_t htab_size(htab *t)
{
        return t->size;
}

bool htab_empty(htab *t)
{
        return (t->size == 0);
}

void htab_add_func(htab *t, function *f, const char * key)
{
        unsigned int index;

        index = hash_function(key) % HTAB_SIZE;

        t->htab[index] = malloc(sizeof(htab_item));
        if (t->htab[index] == NULL)
        {
                // E_INTERNAL
        }
        t->htab[index]->name = malloc(strlen(key) + 1);
        if (t->htab[index]->name == NULL)
        {
                // E_INTERNAL
        }

        t->htab[index]->name_len = strlen(key);

        strcpy(t->htab[index]->name, key);

        //t->htab[index]->used = false;

        t->htab[index]->var_func = FUNCTION;

        // Deep copy
        t->htab[index]->f = *f;

        t->size++;
}

void htab_add_var(htab *t, variable *v, const char * key)
{
        unsigned int index;

        index = hash_function(key) % HTAB_SIZE;

        t->htab[index] = malloc(sizeof(htab_item));
        if (t->htab[index] == NULL)
        {
                // E_INTERNAL
        }
        t->htab[index]->name = malloc(strlen(key) + 1);
        if (t->htab[index]->name == NULL)
        {
                // E_INTERNAL
        }

        t->htab[index]->name_len = strlen(key);

        strcpy(t->htab[index]->name, key);

        //t->htab[index]->used = false;

        t->htab[index]->var_func = VARIABLE;

        // Deep copy
        t->htab[index]->v = *v;

        t->size++;
}

htab_item * get_htab_item(htab *t, const char *key)
{
        return t->htab[hash_function(key) % HTAB_SIZE];
}

void htab_remove(htab *t, const char *s)
{
        unsigned int index;

        index = hash_function(s) % HTAB_SIZE;

        if (t->htab[index] != NULL)
        {
                // Free function or variable info
                free(t->htab[index]->name);
                t->htab[index]->name = NULL;
                free(t->htab[index]);
                t->htab[index] = NULL;
                t->size--;
        }
}

void htab_init(htab **t)
{
        *t = malloc(sizeof **t + (sizeof(htab_item*) * HTAB_SIZE));
        if (*t == NULL)
        {
                //E_INTERN;
        }
        (*t)->size = 0;
        for(int i = 0; i < HTAB_SIZE; i++)
        {
                (*t)->htab[i] = NULL;
        }
}

void htab_free(htab **t)
{
        for (int i = 0; i < HTAB_SIZE; i++)
        {
                if ((*t)->htab[i] != NULL)
                {
                        if ((*t)->htab[i]->var_func == FUNCTION)
                        {
                                if (!(*t)->htab[i]->f.embedded
                                    && (*t)->htab[i]->f.param_names != NULL)
                                {
                                        for (int j = 0; j < (*t)->htab[i]->f.params.top+1; j++)
                                        {
                                                string_free(&(*t)->htab[i]->f.param_names[j]);
                                        }
                                        free((*t)->htab[i]->f.param_names);
                                }
                                stack_free(&(*t)->htab[i]->f.params);
                        }
                        // Free function or variable info
                        free((*t)->htab[i]->name);
                        free((*t)->htab[i]);
                }
        }
        free(*t);
}


int check_func_def(htab *t)
{
        for (int i = 0; i < HTAB_SIZE; i++)
        {
                if (t->htab[i] != NULL
                    && t->htab[i]->var_func == FUNCTION
                    && t->htab[i]->f.declared
                    && !t->htab[i]->f.defined)
                {
                        return E_SEMANTIC_PRG;
                }
        }
        return E_OK;
}

/*
   static unsigned int hash_function(const char * str)
   {
        unsigned int h=0;
        const unsigned char *p;
        for(p=(const unsigned char*)str; *p!='\0'; p++)
        {
                h = 65599*h + *p;
        }
        return h;
   }
 */

static unsigned int hash_function(const char *str)
{
        unsigned int hash = 5381;
        int c;

        while ((c = *str++))
                hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

        return hash;
}
