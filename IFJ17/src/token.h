/**
 * @file token.h
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

#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "reserved.h"
#include "string.h"

/** Enum of all token types which can be read from compiled program
      including key words, data types, operators etc. */
typedef enum
{
        T_NONE = -1,
        TK_AS = 0,    // <--- KEYWORDS_BEGIN
        TK_ASC,
        TK_DECLARE,
        TK_DIM,
        TK_DO,
        TK_DOUBLE,
        TK_ELSE,
        TK_END,
        TK_CHR,
        TK_FUNCTION,
        TK_IF,
        TK_INPUT,
        TK_INTEGER,
        TK_LENGTH,
        TK_LOOP,
        TK_PRINT,
        TK_RETURN,
        TK_SCOPE,
        TK_STRING,
        TK_SUBSTR,
        TK_THEN,
        TK_WHILE,
        TK_AND,
        TK_BOOLEAN,
        TK_CONTINUE,
        TK_ELSEIF,
        TK_EXIT,
        TK_FALSE,
        TK_FOR,
        TK_NEXT,
        TK_NOT,
        TK_OR,
        TK_SHARED,
        TK_STATIC,
        TK_TRUE,
        TO_ASTERISK,  // <--- OPERATORS_BEGIN
        TO_SLASH,
        TO_BACKSLASH,
        TO_LOWER,
        TO_GREATER,
        TO_EQUAL,
        TO_PLUS,
        TO_MINUS,
        TO_LOWER_EQUAL,
        TO_GREATER_EQUAL,
        TO_NOT_EQUAL,
        TO_MINUS_EQUAL,
        TO_PLUS_EQUAL,
        TO_ASTERISK_EQUAL,
        TO_SLASH_EQUAL,
        TO_BACKSLASH_EQUAL,
        TD_OPEN_BRACE, // <--- DELIMETERS_BEGIN
        TD_CLOSE_BRACE,
        TD_SEMICOLON,
        TD_COMMA,
        T_STRING,     // <--- OTHERS
        T_ID,
        T_INT,
        T_FLOAT,
        T_DIVISION,
        T_EOL,
        T_EOF
} TOKEN_TYPE;

/**
 * Indexes where does section begins in enumeration
 */
#define KEYWORDS_BEGIN    0
#define OPERATORS_BEGIN   35
#define DELIMETERS_BEGIN  51

/**
 * @brief Represents token read from standard input
 */
typedef struct
{
        TOKEN_TYPE token_type;      /**< Type of token. */
        union
        {
                char * data_string;
                int data_int;
                double data_float;
        };                          /**< Union of data represented by token. */
} Token;

#endif /* ! TOKEN_H */
