/**
 * @file reserved.h
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

#ifndef RESERVED_H
#define RESERVED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "error.h"
#include "string.h"

/**
 * @defgroup  Reserved keywords / identificators & operators
 *
 * @brief Names of words which are keywords and cannot be used.
 *        They are string literals (placed in read-only) so modifying
 *        would invoke undefined behavior.
 *
 * @{
 */

/**
 * @brief Enumeration of keywords in IFJ17 language
 */
typedef enum
{
        K_NONE = -1,
        K_AS = 0,
        K_ASC,
        K_DECLARE,
        K_DIM,
        K_DO,
        K_DOUBLE,
        K_ELSE,
        K_END,
        K_CHR,
        K_FUNCTION,
        K_IF,
        K_INPUT,
        K_INTEGER,
        K_LENGTH,
        K_LOOP,
        K_PRINT,
        K_RETURN,
        K_SCOPE,
        K_STRING,
        K_SUBSTR,
        K_THEN,
        K_WHILE,
        K_AND,
        K_BOOLEAN,
        K_CONTINUE,
        K_ELSEIF,
        K_EXIT,
        K_FALSE,
        K_FOR,
        K_NEXT,
        K_NOT,
        K_OR,
        K_SHARED,
        K_STATIC,
        K_TRUE,
} KEYWORD_TYPE;

/**
 * @brief Enumeration of operators in IFJ17 language
 */
typedef enum
{
        O_NONE = -1,
        O_ASTERISK = 0,
        O_SLASH,
        O_BACKSLASH,
        O_LOWER,
        O_GREATER,
        O_EQUAL,
        O_PLUS,
        O_MINUS,
        O_LOWER_EQUAL,
        O_GREATER_EQUAL,
        O_NOT_EQUAL
} OPERATOR_TYPE;

/**
 * @brief Enumeration of delimeters in IFJ17 language
 */
typedef enum
{
        D_NONE = -1,
        D_OPEN_BRACE = 0,
        D_CLOSE_BRACE,
        D_SEMICOLON,
        D_COMMA
} DELIMETER_TYPE;

/**
 * @brief Variable types in IFJ17
 */
typedef enum
{
        VT_NONE = -1,
        VT_INT = 0,
        VT_FLOAT,
        VT_STRING
} VAR_TYPE;

/** @brief Array of strings containing reserved (by compiler) keywords */
extern const char*   reserved_keywords[];
/** @brief Array of characters containing reserved (by compiler) operators */
extern const char operators[];
/** @brief Array of characters containing reserved (by compiler) delimeters */
extern const char delimeters[];

/** @brief Gives us number of keywords in array of strings */
#define RESERVED_KEYWORDS_SIZE     (sizeof(reserved_keywords)    / sizeof(const char*))
/** @brief Gives us number of operators in array of characters */
#define OPERATORS_SIZE             (sizeof(operators)            / sizeof(const char ))
/** @brief Gives us number of delimeters in array of characters */
#define DELIMETERS_SIZE            (sizeof(delimeters)           / sizeof(const char ))

/**
 * @brief Returns info if string is reserved keyword
 *
 * @return True - is keyword, else False
 */
bool is_reserved_keyword          (String *s);
/**
 * @brief Returns info if character is operator
 *
 * @return True - is operator, else False
 */
bool is_operator                  (char op);
/**
 * @brief Returns info if character is delimeter
 *
 * @return True - is delimeter, else False
 */
bool is_delimeter                 (char d);

/**
 * @brief Tells us type of keyword which string represents
 *
 * @return Enumeration value representing type of keyword
 */
KEYWORD_TYPE get_keyword_type     (String *s);
/**
 * @brief Tells us type of operator which character represents
 *
 * @return Enumeration value representing type of operator
 */
OPERATOR_TYPE get_operator_type   (char op);
/**
 * @brief Tells us type of delimeter which character represents
 *
 * @return Enumeration value representing type of delimeter
 */
DELIMETER_TYPE get_delimeter_type (char d);

/** @} */

#endif /* ! RESERVED_H */
