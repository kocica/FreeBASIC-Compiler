/**
 * @file error.h
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

#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * @brief Error types returned by compiler
 *
 * If some kind of error appeared one of these code is returned.
 */
typedef enum
{
        E_OK            = 0, /**< Compilation succeded. */
        E_LEXICAL       = 1, /**< Lexical error. */
        E_SYNTACTIC     = 2, /**< Wrong syntax. */
        E_SEMANTIC_PRG  = 3, /**< Undefined/redefined function/variable. */
        E_SEMANTIC_TYPE = 4, /**< Invalid type compatibility (ie wrong count of arguments to function). */
        E_SEMANTIC_OTHR = 6, /**< Other semantic errors. */
        E_INTERN        = 99 /**< Intern error (Memory allocation/file opening failed). */
} ERROR_CODE;

/**
 * @brief Prints Warning: + fmt with parameters to stderr
 */
void WARNING_MSG(const char *fmt, ...);

/**
 * @brief Prints Error: + fmt with parameters to stderr and determines program
 */
void ERROR_MSG(const char *fmt, ...);

#endif /* ! ERROR_H */
