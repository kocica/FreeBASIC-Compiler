/**
 * @file error.c
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

#include "error.h"

void WARNING_MSG(const char *fmt, ...)
{
        va_list arg;
        va_start(arg, fmt);
        fprintf(stderr, "Warning: ");
        vfprintf(stderr, fmt, arg);
        va_end(arg);
}

void ERROR_MSG(const char *fmt, ...)
{
        va_list arg;
        va_start(arg, fmt);
        fprintf(stderr, "ERROR: ");
        vfprintf(stderr, fmt, arg);
        va_end(arg);
}
