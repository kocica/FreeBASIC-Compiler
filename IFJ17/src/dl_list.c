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

#include "dl_list.h"

void DLInitList (dl_list *L)
{
								/// Init pointers to null
								L->First = NULL;
								L->Last  = NULL;
}



void DLDisposeList (dl_list *L)
{
								dl_list_item * tmp;

								while (L->First != NULL)
								{
																tmp = L->First;
																L->First = L->First->rptr;
																htab_free(&tmp->ht);
																free(tmp);
								}

								L->First = L->Last = NULL;
}


void DLAdd (dl_list *L)
{
								dl_list_item * p = malloc(sizeof *p);

								if (p == NULL)
								{
																// E_INTERNAL
																return;
								}

								htab_init(&p->ht);

								if (L->First == NULL)
								{
																L->Last = p;
																L->Last->lptr = NULL;
																L->Last->rptr = NULL;
																L->First = L->Last;
								}
								else
								{
																p->rptr = NULL;
																p->lptr = L->Last;
																L->Last->rptr = p;
																L->Last = p;
								}
}


void DLDeleteLast (dl_list *L)
{
								if (L->First == NULL)
								{
																return;
								}

								dl_list_item * tmp = L->Last;

								if (L->First != L->Last)
								{
																L->Last = L->Last->lptr;
																L->Last->rptr = NULL;
								}
								else
								{
																L->First = NULL;
																L->Last = NULL;
								}

								htab_free(&tmp->ht);
								free(tmp);
}


htab * DLGetLast (const dl_list * L)
{
								return (L->Last == NULL ? NULL : L->Last->ht);
}

htab * DLGetFirst (const dl_list * L)
{
								return (L->First == NULL ? NULL : L->First->ht);
}


htab_item * DLGetFirstFound (const dl_list * L, const char * s)
{
								dl_list_item * item = L->Last;
								htab_item    * tmp  = NULL;

								while (item != NULL && tmp == NULL)
								{
																tmp  = get_htab_item(item->ht, s);
																item = item->lptr;
								}

								return tmp;
}
