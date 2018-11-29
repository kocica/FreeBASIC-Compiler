/**
 * @file parser.h
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

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <ctype.h>

#include "token.h"
#include "scanner.h"
#include "dl_list.h"
#include "stack.h"
#include "embedded_funcs.h"
#include "target_code_gen.h"

/** Actual token got from scanner */
Token token;

/** Instance of string class for easier string operations
    There is saved actual token in scanner */
String token_buffer;

/** Params of functions to check in semantic analyzation */
TStack params;

/** In scanner rows and cols are counted so we can tell user on which line & col
    the error has occured */
extern int row;
extern int col;

/** Types of expressions which can occur in compiled program */
typedef enum
{
        ET_INT = 0,
        ET_FLOAT,
        ET_STRING,
        ET_BOOLEAN,
        ET_ID,
        ET_FUNC_INT,
        ET_FUNC_FLOAT,
        ET_FUNC_STRING,
        ET_EPSYLON,
} EXPR_TYPE_t;

/** Sign of digit expression */
typedef enum
{
        S_NONE = 0,
        S_PLUS,
        S_MINUS
} EXPR_SIGN_t;

/** Checks the implicit conversions */
VAR_TYPE check_e_types(VAR_TYPE first, VAR_TYPE second);

/** Parser called from main which does all the job and returns error code | 0 */
int parser();

#endif /* ! PARSER_H */