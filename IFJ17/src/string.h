/**
 * @file string.h
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

#ifndef STRING_H
#define STRING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "error.h"

/**
 * @brief For easier manipulation of dynamically allocated string
 *        we have created structure which represents one string.
 */
typedef struct
{
        char *pString; /**< Pointer to dyn. allocated memory containing string. */
        size_t act_size; /**< Actual size of string. */
        size_t alloc_size; /**< Size of allocated memory. */
} String;

/**
 * @brief Creates instantion of String
 */
void string_init(String *s);
/**
 * @brief Frees instantion of String
 */
void string_free(String *s);

/**
 * @brief Copies data from string passed as argument
 *        and returns them as new instantion
 *
 * @return New String
 */
String string_copy_ctor(String *from);
/**
 * @brief Moves data from string passed as argument
 *        and returns them as new instantion
 *
 * @return New String
 */
String string_move_ctor(String *from);

/**
 * @brief Compares two strings lexically
 *
 * @return true - strings are same, else false
 */
bool string_compare(const String *s1, const String *s2);

/**
 * @brief Pushes (and resizes if needed) character to string
 */
void string_push_char(String *s, char c);
/**
 * @brief Removes last character from string
 */
void string_pop_char(String *s);
/**
 * @brief Appends string pointed by s2 to String
 */
void string_append_string(String *s1, char *s2);

/**
 * @brief Transforms String characters to lower
 */
void string_to_lower(String *s);
/**
 * @brief Transforms String characters to upper
 */
void string_to_upper(String *s);

/**
 * @brief Tells us info if string is empty
 *
 * @return true - string is empty, else false
 */
bool string_empty(const String *s);
/**
 * @brief Tells us size of string contained in String
 *
 * @return Size of string
 */
size_t string_get_size(const String *s);
/**
 * @brief Gives us pointer to string contained in String
 *
 * @return Poitner to string contained in String
 */
char * string_get_string(const String *s);

/**
 * @brief Clears memory held by string (set to 0) and leaves memory (does not free)
 */
void string_clear(String *s);

/**
 * @brief Sets first char as null terminating
 */
void string_terminate(String *s);

#endif
