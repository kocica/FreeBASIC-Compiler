/**
 * @file embedded_funcs.h
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

#ifndef EMBEDDED_FUNCS_H
#define EMBEDDED_FUNCS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "symtable.h"
#include "stack.h"

/**
 * @defgroup Embedded functions
 *
 * @brief Functions which are internally supported by compiler
 *
 * !!! These functions were implemented for constant expressions, it means
 *       for optimalization !!!
 * @{
 */

/**
 * @brief Calculates length of string passed as argument
 *
 * @return Length of string
 */
int Length(char *s);

/**
 * @brief Cuts part of string from i-th position and cuts n-characters
 *
 * @return Substring of string s
 */
char * SubStr(char *s, int i, int n);

/**
 * @brief If index is not outside of bounds returns ASCII value of
 *        character on i-th pos, else 0
 *
 * @return ASCII value of character or 0
 */
int Asc(char *s, int i);

/**
 * @brief Dynamically creates string of size 2, where first character
 *        is character passed as argument (if i hasnt ASCII value behavior is undefined)
 *
 * @return String of size 2 with character passed as arg and null terminating character
 */
char * Chr(int i);

/** } */

/**
 * @brief Adds all built-in function names to first htab in list -- to global scope htab
 *        also saves return types and param types for semantic analyzation
 *
 */
void add_funcs_to_htab(htab * ht);

#endif /* ! EMBEDDED_FUNCS_H */