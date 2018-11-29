/**
 * @file target_code_gen.h
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

#ifndef TARGET_CODE_GEN_H
#define TARGET_CODE_GEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
    Since instruction are printed to stdout and
      we dont want them to print if an error has occured,
      weve got to store them somewhere.
    Weve chosen the double linked list.
    To be honest idk why its double linked, never mind tho
 */

/** Represents one instruction in list of all instructions */
typedef struct instr
{
        char * str;
        struct instr * lptr;
        struct instr * rptr;
} instr;

/** Typicall double linked list, pointers to first and last element */
typedef struct
{
        instr * First;
        instr * Last;
} instr_generator;

/** Prints all instruction to stdout after succesfull compilation */
void INSTR_PRINT(instr_generator *instr_gen);

/** Inist double linked list */
void INSTR_GEN_INIT(instr_generator *instr_gen);

/** Frees resources of double linked list */
void INSTR_GEN_FREE(instr_generator *instr_gen);

/** Saves first part of instruction */
void INSTR_GEN(instr_generator *instr_gen, const char *instr_string);

/** Appends string to (last) already saved instruction */
void INSTR_GEN_ADD_S(instr_generator *instr_gen, const char *instr_string);

/** Appends integer to (last) already saved instruction */
void INSTR_GEN_ADD_I(instr_generator *instr_gen, const int instr_int);

/** Appends float to (last) already saved instruction */
void INSTR_GEN_ADD_F(instr_generator *instr_gen, const float instr_float);

#endif /* ! TARGET_CODE_GEN_H */