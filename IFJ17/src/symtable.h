/**
 * @file symtable.h
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

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "error.h"
#include "reserved.h"
#include "stack.h"
#include "string.h"

/** Since hash table is implemented to hold informations about functions and variables, there
    are two structures, variable and function, each of them hold important informations about them */

/** To check function we need to know if its embedded one or not,
    then we need to know if it has been already defined / declared.
    Also return type and params (both types and names) are fine to remember */
typedef struct
{
        bool embedded;
        bool declared;
        bool defined;
        VAR_TYPE return_type;
        TStack params;
        String * param_names;
} function;

/** When resizing stack of params, we wont do it by one, right ? */
#define PARAM_RESIZE_CHUNK 10

/** Types of scopes in compiled program */
typedef enum
{
        S_LOCAL,
        S_TEMP,
        //S_GLOBAL // Extension GLOBAL
} SCOPE_TYPE_t;

/** What do we need to remember about variables ?
    I guess its type, in which scope it was declared,
    and save actual counter of variables to avoid
    name collision */
typedef struct
{
        VAR_TYPE type;
        SCOPE_TYPE_t scope;
        int n;
} variable;

/** We wont do two hash tables for variables and functions.
    This will help us detect if the item in htab is function or var */
typedef enum
{
        VARIABLE = 0,
        FUNCTION = 1
} VAR_FUNC;

/**
 * @brief Represents one hash table item, saves information if its var or function
 *        Then union with func/var structure, to save space cuz 8GB is not enough
 */
typedef struct
{
        VAR_FUNC var_func;
        union
        {
                variable v;
                function f;
        };

        //bool used; // For ununsed variables warning -- we wont get any points for that so remove it
        char * name; /**< Hashed key. */
        size_t name_len; /**< Length if heshed key */
} htab_item;

/**
 * @brief Represents whole hash table
 */
typedef struct
{
        size_t size; /**< Size of hash table. */
        htab_item *htab[]; /**< Hash table represented as flexible array member. */
} htab;

/**
 * @brief Returns size of hash table
 *
 * @param t [in] Pointer to hash table
 *
 * @return Size of hash table
 */
size_t htab_size(htab *t);

/**
 * @brief Tells us whether hash table is empty
 *
 * @param t [in] Pointer to hash table
 *
 * @return true if empty, else false
 */
bool htab_empty(htab *t);

/**
 * @brief Adds new function hashtable
 *
 * @param t [in] Pointer to hash table
 * @param f [in] Pointer to function info
 * @param key [in] Hash key
 */
void htab_add_func(htab *t, function *f, const char *key);

/**
 * @brief Adds new variable hashtable
 *
 * @param t [in] Pointer to hash table
 * @param v [in] Pointer to variable info
 * @param key [in] Hash key
 */
void htab_add_var(htab *t, variable *v, const char *key);

/**
 * @brief Returns data on index of hashed key
 *
 * @param t [in] Pointer to hash table
 * @param key [in] Hash key
 */
htab_item * get_htab_item(htab *t, const char *key);

/**
 * @brief Removes item pointed by parameter i
 *
 * @param t [in] Pointer to hash table
 * @param s [in] Pointer to hash item to remove
 */
void htab_remove(htab *t, const char *s);

/**
 * @brief Initialize new hash table
 *
 * @param t [out] Hash table to init
 */
void htab_init(htab **t);

/**
 * @brief Frees hash table
 *
 * @param t [out] Pointer to hash table
 */
void htab_free(htab **t);

/**
 * @brief Checks if all declared function are defined as well
 *
 * @param t [out] Pointer to hash table
 */
int check_func_def(htab *t);

#endif /* ! SYMTABLE_H */
