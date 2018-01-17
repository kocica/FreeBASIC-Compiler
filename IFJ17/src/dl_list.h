/**
 * @file dl_list.h
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
 * @date      28/10/2017
 */

#ifndef DL_LIST_H
#define DL_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "symtable.h"

/** Represents one hash table in list of hast tables,
    hash tables are added when new scope is needed, ie. function body has his own HT

    Contains reference to htab and both left and right htabs */
typedef struct dl_list_item
{
        htab * ht;
        struct dl_list_item * lptr;
        struct dl_list_item * rptr;
} dl_list_item;

/** Hash table double linked list */
typedef struct
{
        dl_list_item * First;
        dl_list_item * Last;
} dl_list;

/** List initialization, setting pointers to NULL */
void DLInitList (dl_list *);

/** Free all htabs and whole list */
void DLDisposeList (dl_list *);

/** Adding new htab to list */
void DLAdd (dl_list *);

/** Deleting last htab, ie. if we are leaving function body */
void DLDeleteLast (dl_list *);

/** Returning last htab in list, actual scope */
htab * DLGetLast (const dl_list *);

/** Returning first htab in list, global scope */
htab * DLGetFirst (const dl_list *);

/** Returning first found item with same key,
      goes from last to first (global) htab -- scope */
htab_item * DLGetFirstFound (const dl_list *, const char *);

#endif /* ! DL_LIST_H */
