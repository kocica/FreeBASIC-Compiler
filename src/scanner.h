/**
 * @file scanner.h
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

#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "error.h"
#include "reserved.h"
#include "token.h"
#include "string.h"

/** States of final state machine */
typedef enum
{
        S_INIT,
        // Extension BASE
        S_BASE,
        S_BASE_OCTAL,
        S_BASE_HEXA,
        S_BASE_BINARY,
        // ! Extension BASE
        S_ONE_LINE_COMMENT,
        S_MULTIPLE_LINES_COMMENT,
        S_MULTIPLE_LINES_COMMENT_END,
        S_ID,
        S_ID_KEYWORD,
        S_SLASH,
        S_STRING_BACKSLASH,
        S_STRING_BEGIN,
        S_STRING,
        S_NUMBER,
        S_FLOAT,
        S_EXP
} SCANNER_STATE;

/** If we got final state of FSM, return that */
#define FINAL_STATE 0
#define RETURN_FINAL_STATE do { return FINAL_STATE; } while(0)

/** There are letters of token stored */
extern String token_buffer;

/** Token which is sent from scanner to parser */
extern Token token;

/**
 * @brief Get token from file.
 *
 * Scanning source code and extracting all tokens one by one
 *
 * @return Token value
 */
int get_token();

/**
 * @brief Performs lexical analyzation process during compilation.
 *
 * Scans source code from standard input, removes comments and extract tokens
 * from source code, pushing them to table.
 */
#ifdef SCANNER_DEBUG
void scanner();
#endif
/**
 * @brief Prints type of token.
 *
 * If we are debugging we want to know type of token instead of its code.
 */
#ifdef SCANNER_DEBUG
void print_token_type(TOKEN_TYPE tt);
#endif

/** @} */

#endif /* ! SCANNER_H */
