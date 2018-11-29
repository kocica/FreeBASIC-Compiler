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
 * @author(s) Matej Knazik (xknazi00@fit.vutbr.cz), FIT
 * @date      24.9.2017
 */

/**
 * TODO
 *
 * 1) Empty program - Semantic or OK ?                                                     -- Done
 * 2) Program without scope correct ?                                                      -- Done
 * 3.1) Return in if-else & do-while                                                       -- Done
 * 3.2) Function decl and def has different param names                                    -- Done
 * 4) Optimal size of htab                                                                 -- Who cares tho, server has enough memory
 * 5) May i write if-else without else ? ie. if 1>0 then <EOL> print !"42" <EOL> end if    -- Done
 * 6) <EOF> after end scope ? may the code continue there ?                                -- Done
 * 7) + (concat) on strings ? ie. id = !"ah" + !"oj"                                       -- Done
 * 8) Return types of declaration and definition of func doesnt fit                        -- (Forum: https://wis.fit.vutbr.cz/FIT/st/phorum-msg-show.php?id=49717)
 */

#include "parser.h"

/** If we need to accept token from scanner but we also need to hold info,
    copy it here */
static String string;
/** List of hash tables, first is global scope, last is actual scope */
static dl_list * htab_list;
/** List with generated instrucions */
static instr_generator * instr_gen;
/** If we need to save return type of function, cuz passing as parameter is too hard */
static VAR_TYPE vt;
/** Type of lvalue */
static VAR_TYPE assign_type;
/** Expressions parsing variables */
static EXPR_TYPE_t e_type; static EXPR_SIGN_t e_sign = S_NONE;
/** Boolean help variables */
static bool func_params, substr, in_func, concat;
/** Tells us type of expression */
static VAR_TYPE expr_type, expr_type_prev;
/** Counters which are appended and increased when variable is declared */
static int int_counter, float_counter, string_counter, stmnt_counter, var_counter;
/** Buffer for generated instructions */
static String gen_buffer;
/** We have to store function param names */
static String * param_names; static int param_counter;
static TStackString strstack;

/** Gets token from scanner and checks if lexical error has occured */
#define GET_TOKEN do { \
                if (get_token() == E_LEXICAL) \
                { \
                        return E_LEXICAL; \
                } \
} while(0)

/** Calls function F and immediatelly check for return type */
#define HANDLE_CALL_F(F) do { \
                int ec = F(); \
                if (ec != E_OK) \
                { \
                        return ec; \
                } \
} while(0)

/** Accepts expected token or return syntax error */
#define accept(act_token) do { \
                if (token.token_type == act_token) \
                { \
                        GET_TOKEN; \
                } \
                else \
                { \
                        return E_SYNTACTIC; \
                } \
} while(0)

/** Checks expression type */
#define check_e() do { \
                if ((expr_type = check_e_types(expr_type_prev, expr_type)) == VT_NONE) \
                { \
                        ERROR_MSG("Wrong types in E\n"); \
                        return E_SEMANTIC_TYPE; \
                } \
                e_type = expr_type; \
                switch (e_type) \
                { \
                case VT_INT: \
                        int_counter ++; \
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT"); \
                        INSTR_GEN_ADD_I(instr_gen, int_counter); \
                        INSTR_GEN_ADD_S(instr_gen, string_get_string(&gen_buffer)); \
                        string_terminate(&gen_buffer); \
                        char * temp_string = malloc(100); \
                        sprintf(temp_string, " GF@E_INT%d", int_counter); \
                        string_append_string(&gen_buffer, temp_string); \
                        free(temp_string); \
                        break; \
                case VT_FLOAT: \
                        float_counter ++; \
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT"); \
                        INSTR_GEN_ADD_I(instr_gen, float_counter); \
                        INSTR_GEN_ADD_S(instr_gen, string_get_string(&gen_buffer)); \
                        string_terminate(&gen_buffer); \
                        temp_string = malloc(100); \
                        sprintf(temp_string, " GF@E_FLOAT%d", float_counter); \
                        string_append_string(&gen_buffer, temp_string); \
                        free(temp_string); \
                        break; \
                case VT_STRING: \
                        string_counter ++; \
                        if (concat) \
                        { \
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING"); \
                                INSTR_GEN_ADD_I(instr_gen, string_counter); \
                                expr_type = VT_STRING; \
                        } \
                        else \
                        { \
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT"); \
                                INSTR_GEN_ADD_I(instr_gen, int_counter); \
                                expr_type = VT_INT; \
                        } \
                        INSTR_GEN_ADD_S(instr_gen, string_get_string(&gen_buffer)); \
                        string_terminate(&gen_buffer); \
                        temp_string = malloc(100); \
                        sprintf(temp_string, " GF@E_STRING%d", string_counter); \
                        string_append_string(&gen_buffer, temp_string); \
                        free(temp_string); \
                        break; \
                default: \
                        break; \
                } \
} while (0)

/**
 * @brief Static funtion declaration used in parser
 *
 * These functions are declared as static, cause they are called
 * only from parser
 */
static String * string_conv_spaces(char * str);


/** LL-gramatics implementation */
static int S();
static int Scope();

static int declaration_list();
static int function_decl();
static int data_type();

static int print_list();

static int param();
static int statement_list();

static bool isExpression();
static bool isArith();
static bool isTerm_intd();
static bool isTerm();

static int E();
static int arith();
static int term_intd();
static int factor();
static int term();

static int value();
static int call();
static int assign();
static int call_assign();

static int check_func_signature(htab_item *hti);


static int S()
{
        do
        {
                switch (token.token_type)
                {
                case T_EOF:
                {
                        return E_SYNTACTIC; // TODO no Scope error?
                }

                case T_EOL:
                {
                        accept(T_EOL);
                        HANDLE_CALL_F (S);
                        return E_OK;
                }

                //case TK_DIM:
                case TK_DECLARE:
                case TK_FUNCTION:
                {
                        HANDLE_CALL_F (declaration_list);
                        HANDLE_CALL_F (S);
                        return E_OK;
                }

                case TK_SCOPE:
                {
                        INSTR_GEN(instr_gen, "LABEL $$main");
                        INSTR_GEN(instr_gen, "CREATEFRAME");

                        // Check if all declared functions are defined as well, otherwise return error 3
                        htab * ht = DLGetFirst(htab_list);
                        if (check_func_def(ht) != E_OK)
                        {
                                return E_SEMANTIC_PRG;
                        }
                        break;
                }

                default:
                {
                        return E_SYNTACTIC;
                }
                }
        } while (token.token_type != TK_SCOPE && token.token_type != T_EOF);

        accept(TK_SCOPE);
        accept(T_EOL);
        HANDLE_CALL_F (Scope);
        return E_OK;
}


static int Scope()
{
        instr * scope_beg = instr_gen->Last;

        do
        {
                switch (token.token_type)
                {
                case T_EOL:
                {
                        accept(T_EOL);
                        break;
                }

                case T_ID:
                case TK_PRINT:
                case TK_INPUT:
                case TK_IF:
                case TK_DO:
                case TK_DIM:
                {
                        HANDLE_CALL_F (statement_list);
                        break;
                }

                case TK_END:
                {
                        //DLDeleteLast(htab_list);
                        break;
                }

                default:
                {
                        return E_SYNTACTIC;
                }
                }
        } while (token.token_type != TK_END && token.token_type != T_EOF);

        accept(TK_END);
        accept(TK_SCOPE);
        while (token.token_type == T_EOL)
        {
                accept(T_EOL);
        }
        accept(T_EOF);

        instr * tmp = instr_gen->Last;
        instr * tmp_next = scope_beg->rptr;
        instr_gen->Last = scope_beg;


        for (int i = 0; i <= int_counter; i++)
        {
                INSTR_GEN(instr_gen, "DEFVAR GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, i);
                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, i);
                INSTR_GEN_ADD_S(instr_gen, " int@0");
        }
        for (int i = 0; i <= float_counter; i++)
        {
                INSTR_GEN(instr_gen, "DEFVAR GF@E_FLOAT");
                INSTR_GEN_ADD_I(instr_gen, i);
                INSTR_GEN(instr_gen, "MOVE GF@E_FLOAT");
                INSTR_GEN_ADD_I(instr_gen, i);
                INSTR_GEN_ADD_S(instr_gen, " float@0.0");
        }

        for (int i = 0; i <= string_counter; i++)
        {
                INSTR_GEN(instr_gen, "DEFVAR GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, i);
                INSTR_GEN(instr_gen, "MOVE GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, i);
                INSTR_GEN_ADD_S(instr_gen, " string@");
        }
        for (int i = 0; i <= strstack.top; i++)
        {
                INSTR_GEN(instr_gen, strstack.arr[i]);
        }

        instr_gen->Last->rptr = tmp_next;
        instr_gen->Last = tmp;
        int_counter = 0; float_counter = 0; string_counter = 0;

        return E_OK;
}

static int declaration_list()
{
        switch (token.token_type)
        {
        case TK_FUNCTION:
        {
                DLAdd(htab_list);
                stack_init(&params);
                param_names = malloc(sizeof *param_names * PARAM_RESIZE_CHUNK);
                param_counter = 0;
                func_params = true;
                HANDLE_CALL_F (function_decl);
                func_params = false;
                accept(T_EOL);

                // Get global scope
                htab * ht = DLGetFirst(htab_list);
                htab_item *hti = get_htab_item(ht, string_get_string(&string));

                if (hti == NULL)
                {
                        function f = { .defined = true, .declared = false, .embedded = false, .return_type = vt };
                        f.params.top = params.top;
                        f.params.allocated = params.allocated;
                        f.params.arr = params.arr;
                        f.param_names = param_names;
                        htab_add_func(ht, &f, string_get_string(&string));
                        stack_clear(&params);
                }
                else if (hti->var_func == VARIABLE)
                {
                        ERROR_MSG("Parser: Redefinition: Symbol %s is already defined as variable\n", string_get_string(&string));
                        return E_SEMANTIC_PRG;
                }
                else if (hti->var_func == FUNCTION && hti->f.defined)
                {
                        ERROR_MSG("Parser: Redefinition: Symbol %s is already defined as function\n", string_get_string(&string));
                        return E_SEMANTIC_PRG;
                }
                else
                {
                        int ec = E_OK;
                        if (hti->f.return_type != vt)
                        {
                                return E_SEMANTIC_PRG; // TODO : TYPE or PRG semantic error
                        }
                        if ((ec = check_func_signature(hti)) != E_OK)
                        {
                                ERROR_MSG("Parser: Function %s wrong signature\n", hti->name);
                                return ec;
                        }
                        hti->f.defined = true;
                        hti->f.param_names = param_names;
                        stack_free(&params);
                }

                INSTR_GEN(instr_gen, "LABEL $");
                INSTR_GEN_ADD_S(instr_gen, string_get_string(&string));
                INSTR_GEN(instr_gen, "PUSHFRAME");
                INSTR_GEN(instr_gen, "DEFVAR LF@%retval");
                hti = get_htab_item(ht, string_get_string(&string));
                switch(hti->f.return_type)
                {
                case VT_INT:
                        INSTR_GEN(instr_gen, "MOVE LF@%retval int@0");
                        break;
                case VT_FLOAT:
                        INSTR_GEN(instr_gen, "MOVE LF@%retval float@0.0");
                        break;
                case VT_STRING:
                        INSTR_GEN(instr_gen, "MOVE LF@%retval string@");
                        break;
                default:
                        break;
                }

                INSTR_GEN(instr_gen, "CREATEFRAME");

                in_func = true;
                do
                {
                        if (token.token_type == TK_DIM)
                        {
                                HANDLE_CALL_F (declaration_list);
                        }
                        else if (token.token_type == TK_END)
                        {
                                break;
                        }
                        else
                        {
                                HANDLE_CALL_F (statement_list);
                        }
                } while (token.token_type != TK_END);
                in_func = false;

                INSTR_GEN(instr_gen, "POPFRAME");
                INSTR_GEN(instr_gen, "RETURN");

                accept(TK_END);
                accept(TK_FUNCTION);
                accept(T_EOL);

                DLDeleteLast(htab_list);

                break;
        }

        case TK_DIM:
        {
                accept(TK_DIM);

                if (token.token_type == T_ID)
                {
                        string_clear(&string);
                        string_append_string(&string, token.data_string);

                }

                accept(T_ID);
                accept(TK_AS);

                int type = data_type();
                if (type == VT_NONE)
                {
                        return E_SYNTACTIC;
                }
                GET_TOKEN;

                char * tmp_str = malloc(strlen(string_get_string(&string))+1);
                strcpy(tmp_str, string_get_string(&string));

                htab * ht = DLGetLast(htab_list);
                htab_item *hti = get_htab_item(ht, tmp_str);

                if (hti == NULL)
                {
                        variable v = { .type = type, .scope = S_TEMP, .n = var_counter };
                        char *tmp_s = malloc(100);
                        *tmp_s = 0;
                        strcpy(tmp_s, "DEFVAR GF@");
                        strcat(tmp_s, tmp_str);
                        char *tmp_s2 = malloc(10);
                        *tmp_s2 = 0;
                        sprintf(tmp_s2, "%d", var_counter);
                        strcat(tmp_s, tmp_s2);
                        free(tmp_s2);
                        stack_push_string(&strstack, tmp_s);


                        switch (type)
                        {
                        case VT_STRING:
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@");
                                INSTR_GEN_ADD_S(instr_gen, tmp_str);
                                INSTR_GEN_ADD_I(instr_gen, var_counter);
                                INSTR_GEN_ADD_S(instr_gen, " string@");
                                break;
                        }
                        case VT_FLOAT:
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@");
                                INSTR_GEN_ADD_S(instr_gen, tmp_str);
                                INSTR_GEN_ADD_I(instr_gen, var_counter);
                                INSTR_GEN_ADD_S(instr_gen, " float@0.0");
                                break;
                        }
                        case VT_INT:
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@");
                                INSTR_GEN_ADD_S(instr_gen, tmp_str);
                                INSTR_GEN_ADD_I(instr_gen, var_counter);
                                INSTR_GEN_ADD_S(instr_gen, " int@0");
                                break;
                        }
                        }

                        var_counter++;
                        htab_add_var(ht, &v, tmp_str);
                }
                else
                {
                        ERROR_MSG("Parser: Redefinition: Variable %s already defined\n", tmp_str);
                        return E_SEMANTIC_PRG;
                }

                HANDLE_CALL_F (assign);
                hti = get_htab_item(ht, tmp_str);

                switch (e_type)
                {
                case ET_STRING:
                {
                        if (type != VT_STRING)
                        {
                                ERROR_MSG("Parser: Wrong assignment init\n");
                                return E_SEMANTIC_TYPE;
                        }
                        INSTR_GEN(instr_gen, "MOVE GF@");
                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                        INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING");
                        INSTR_GEN_ADD_I(instr_gen, string_counter);
                        break;
                }
                case ET_INT:
                {
                        if (type != VT_INT && type != VT_FLOAT)
                        {
                                ERROR_MSG("Parser: Wrong assignment init\n");
                                return E_SEMANTIC_TYPE;
                        }
                        else if (type == VT_FLOAT)
                        {
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                INSTR_GEN(instr_gen, "MOVE GF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                                INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                        }
                        else
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                                INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                        }
                        break;
                }
                case ET_FLOAT:
                {
                        if (type != VT_INT && type != VT_FLOAT)
                        {
                                ERROR_MSG("Parser: Wrong assignment init\n");
                                return E_SEMANTIC_TYPE;
                        }
                        else if (type == VT_INT)
                        {
                                INSTR_GEN(instr_gen, "FLOAT2INT GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN(instr_gen, "MOVE GF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                                INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                        }
                        else
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                                INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                        }
                        break;
                }
                case ET_EPSYLON:
                {
                        break;
                }
                default:
                {
                        ERROR_MSG("Parser: Wrong assignment init\n");
                        return E_SEMANTIC_TYPE;
                }
                }

                free(tmp_str);
                break;
        }

        case TK_DECLARE:
        {
                stack_init(&params);
                param_names = malloc(sizeof *param_names * PARAM_RESIZE_CHUNK);
                param_counter = 0;

                accept(TK_DECLARE);
                HANDLE_CALL_F (function_decl);
                accept(T_EOL);

                // Get global scope
                htab * ht = DLGetFirst(htab_list);
                htab_item *hti = get_htab_item(ht, string_get_string(&string));
                if (hti == NULL)
                {
                        function f = { .declared = true, .embedded = false, .defined = false, .return_type = vt };
                        f.params.top = params.top;
                        f.params.allocated = params.allocated;
                        f.params.arr = params.arr;
                        for (int i = 0; i < param_counter; i++)
                        {
                                string_free(&param_names[i]);
                        }
                        htab_add_func(ht, &f, string_get_string(&string));
                        break;
                }
                else if (hti->var_func == VARIABLE)
                {
                        ERROR_MSG("Parser: Redefinition: Symbol %s is already defined as variable\n", string_get_string(&string));
                        return E_SEMANTIC_PRG;
                }
                else if (hti->var_func == FUNCTION && hti->f.declared)
                {
                        int ec = E_OK;
                        if ((ec = check_func_signature(hti)) != E_OK)
                        {
                                return ec; /// Wrong params, return semantic error
                        }
                }
                else if (hti->var_func == FUNCTION && hti->f.defined)
                {
                        ERROR_MSG("Parser: Declaration: Function %s is already defined.\n", string_get_string(&string));
                        return E_SEMANTIC_PRG;
                        // TODO : Check correctness of returned error
                }
                else
                {
                        hti->f.declared = true;
                }

                stack_free(&params);
                break;
        }

        default:
        {
                return E_SYNTACTIC;
        }
        }
        return E_OK;
}


static int function_decl()
{
        accept(TK_FUNCTION);

        if (token.token_type == T_ID)
        {
                string_clear(&string);
                string_append_string(&string, token.data_string);
        }
        accept(T_ID);

        accept(TD_OPEN_BRACE);
        HANDLE_CALL_F (param);
        accept(TD_CLOSE_BRACE);

        accept(TK_AS);

        int type = data_type();
        if (type == VT_NONE)
        {
                return E_SYNTACTIC;
        }
        vt = type;

        GET_TOKEN;

        return E_OK;
}


static int param()
{
        String s;
        if (token.token_type == T_ID) // ID as ValueType rule
        {
                string_init(&s);
                string_append_string(&s, token.data_string);

                accept(T_ID);
                accept(TK_AS);

                int type = data_type();
                if (type == VT_NONE)
                {
                        return E_SYNTACTIC;
                }
                GET_TOKEN;

                if (!strcmp(string_get_string(&string), string_get_string(&s)))
                {
                        return E_SEMANTIC_PRG;
                }

                htab_item *hti = DLGetFirstFound(htab_list, string_get_string(&s));
                if (hti != NULL)
                {
                        return E_SEMANTIC_PRG;
                }

                if (func_params)
                {
                        htab * ht = DLGetLast(htab_list);
                        hti = get_htab_item(ht, string_get_string(&s));

                        if (hti == NULL)
                        {
                                variable v = { .type = type, .scope = S_LOCAL };

                                htab_add_var(ht, &v, string_get_string(&s));
                        }
                        else
                        {
                                ERROR_MSG("Parser: Param '%s' redefinition\n", string_get_string(&s));
                                return E_SEMANTIC_PRG;
                        }
                }

                if ((param_counter % PARAM_RESIZE_CHUNK) == 0 && param_counter)
                {
                        String * tmp = malloc(sizeof *tmp * (params.top + 1 + PARAM_RESIZE_CHUNK));
                        for (int i = 0; i < params.top+1; i++)
                        {
                                tmp[i] = param_names[i];
                        }
                        free(param_names);
                        param_names = tmp;
                }
                stack_push(&params, type);
                string_init(&param_names[param_counter]);
                string_append_string(&param_names[param_counter++], string_get_string(&s));
                string_free(&s);

                if (token.token_type == TD_COMMA)
                {
                        accept(TD_COMMA);
                        if (token.token_type == TD_CLOSE_BRACE)
                        {
                                return E_SYNTACTIC;
                        }
                        HANDLE_CALL_F(param);
                }

                return E_OK;
        }
        else if (token.token_type == TD_CLOSE_BRACE) // Epsylon rule
        {
                return E_OK;
        }

        return E_SYNTACTIC;
}


static int data_type()
{
        switch(token.token_type)
        {
        case TK_INTEGER:
        {
                return VT_INT;
        }
        case TK_DOUBLE:
        {
                return VT_FLOAT;
        }
        case TK_STRING:
        {
                return VT_STRING;
        }
        default:
        {
                return VT_NONE;
        }
        }
}

static bool isExpression() {
        switch(token.token_type) {
        case TO_EQUAL:
        case TO_NOT_EQUAL:
        case TO_LOWER:
        case TO_LOWER_EQUAL:
        case TO_GREATER:
        case TO_GREATER_EQUAL:
                return true;
                break;
        default:
                return false;
                break;
        }
}

static bool isArith() {
        switch(token.token_type) {
        case TO_MINUS:
        case TO_PLUS:
                return true;
                break;
        default:
                return false;
                break;
        }
}

static bool isTerm_intd() {
        switch(token.token_type) {
        case TO_BACKSLASH:
                return true;
                break;
        default:
                return false;
                break;
        }
}

static bool isTerm() {
        switch(token.token_type) {
        case TO_ASTERISK:
        case T_DIVISION:
                return true;
                break;
        default:
                return false;
                break;
        }
}

#define do_op(op) do { \
                expr_type_prev = expr_type; \
                accept(token.token_type); \
                tmp_s = malloc(strlen(string_get_string(&gen_buffer))+1); \
                strcpy(tmp_s, string_get_string(&gen_buffer)); \
                string_terminate(&gen_buffer); \
                switch(expr_type_prev) \
                { \
                case VT_INT: \
                        int_counter++; \
                        break; \
                case VT_FLOAT: \
                        float_counter++; \
                        break; \
                case VT_STRING: \
                        string_counter++; \
                        break; \
                default: \
                        break; \
                } \
                HANDLE_CALL_F(arith); \
                if (expr_type_prev == VT_INT && expr_type == VT_FLOAT) \
                { \
                        float_counter++; \
                        INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT"); \
                        INSTR_GEN_ADD_I(instr_gen, float_counter); \
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT"); \
                        INSTR_GEN_ADD_I(instr_gen, int_counter - 1); \
                        string_terminate(&gen_buffer); \
                        char * gen = malloc(100); *gen = 0; \
                        sprintf(gen, " GF@E_FLOAT%d", float_counter); \
                        strcpy(string_get_string(&gen_buffer), gen); \
                        free(gen); \
                        float_counter--; \
                } \
                else if (expr_type_prev == VT_FLOAT && expr_type == VT_INT) \
                { \
                        INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT"); \
                        INSTR_GEN_ADD_I(instr_gen, float_counter); \
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT"); \
                        INSTR_GEN_ADD_I(instr_gen, int_counter); \
                        string_terminate(&gen_buffer); \
                        char * gen = malloc(100); *gen = 0; \
                        sprintf(gen, " GF@E_FLOAT%d", float_counter - 1); \
                        strcat(string_get_string(&gen_buffer), gen); \
                        expr_type = ET_FLOAT; \
                        free(gen); \
                } \
                else \
                { \
                        strcpy(string_get_string(&gen_buffer), tmp_s); \
                } \
                t1 = malloc(100); \
                if (expr_type == VT_FLOAT || expr_type_prev == VT_FLOAT) \
                { \
                        sprintf(t1, " GF@E_FLOAT%d", float_counter); \
                        expr_type = VT_FLOAT; \
                } \
                else if (expr_type == VT_STRING && expr_type_prev == VT_STRING) \
                { \
                        sprintf(t1, " GF@E_STRING%d", string_counter); \
                        expr_type = VT_STRING; \
                } \
                else \
                { \
                        sprintf(t1, " GF@E_INT%d", int_counter); \
                        expr_type = VT_INT; \
                } \
                strcat(gen_buffer.pString, t1); \
                free(t1); \
                free(tmp_s); \
                INSTR_GEN(instr_gen, op); \
} while (0)

static int E()
{
        e_sign = S_NONE;
        string_terminate(&gen_buffer);

        //    E         : arith ((comparsion_op) arith)*
        //    arith     : term_intd ((PLUS or MINUS) term_intd)*
        //    term_intd : term (INT_DIV term)*
        //    term      : factor ((MUL or DIV) factor)*
        //    factor    : INTEGER or (LBRACE E RBRACE) or FLOAT or Variable or string or function

        HANDLE_CALL_F(arith);

        char * tmp_s;
        char * t1;

        while (isExpression()) {
                switch(token.token_type) {
                case TO_EQUAL:
                        do_op("EQ");
                        check_e();
                        if (expr_type == VT_FLOAT)
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                        }
                        e_type = ET_BOOLEAN;
                        break;
                case TO_NOT_EQUAL:
                        do_op("EQ");
                        check_e();
                        if (expr_type == VT_FLOAT)
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                        }
                        INSTR_GEN(instr_gen, "NOT GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        e_type = ET_BOOLEAN;
                        break;
                case TO_LOWER:
                        do_op("LT");
                        check_e();
                        if (expr_type == VT_FLOAT)
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                        }
                        e_type = ET_BOOLEAN;
                        break;
                case TO_LOWER_EQUAL:
                        expr_type_prev = expr_type;
                        accept(token.token_type);
                        tmp_s = malloc(strlen(string_get_string(&gen_buffer))+1);
                        strcpy(tmp_s, string_get_string(&gen_buffer));
                        string_terminate(&gen_buffer);
                        switch(expr_type_prev)
                        {
                        case VT_INT:
                                int_counter++;
                                break;
                        case VT_FLOAT:
                                float_counter++;
                                break;
                        case VT_STRING:
                                string_counter++;
                                break;
                        default:
                                break;
                        }
                        HANDLE_CALL_F(arith);
                        if (expr_type_prev == VT_INT && expr_type == VT_FLOAT)
                        {
                                float_counter++;
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter - 1);
                                string_terminate(&gen_buffer);
                                char * gen = malloc(100); *gen = 0;
                                sprintf(gen, " GF@E_FLOAT%d", float_counter);
                                strcpy(string_get_string(&gen_buffer), gen);
                                free(gen);
                                float_counter--;
                        }
                        else if (expr_type_prev == VT_FLOAT && expr_type == VT_INT)
                        {
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                string_terminate(&gen_buffer);
                                char * gen = malloc(100); *gen = 0;
                                sprintf(gen, " GF@E_FLOAT%d", float_counter - 1);
                                strcat(string_get_string(&gen_buffer), gen);
                                expr_type = ET_FLOAT;
                                free(gen);
                        }
                        else
                        {
                                strcpy(string_get_string(&gen_buffer), tmp_s);
                        }
                        t1 = malloc(100);
                        if (expr_type == VT_FLOAT || expr_type_prev == VT_FLOAT)
                        {
                                sprintf(t1, " GF@E_FLOAT%d", float_counter);
                                expr_type = VT_FLOAT;
                        }
                        else if (expr_type == VT_STRING && expr_type_prev == VT_STRING)
                        {
                                sprintf(t1, " GF@E_STRING%d", string_counter);
                                expr_type = VT_STRING;
                        }
                        else
                        {
                                sprintf(t1, " GF@E_INT%d", int_counter);
                                expr_type = VT_INT;
                        }
                        strcat(gen_buffer.pString, t1);
                        free(t1);
                        free(tmp_s);
                        char * tmp_eq = malloc(strlen(gen_buffer.pString) + 1);
                        strcpy(tmp_eq, gen_buffer.pString);
                        INSTR_GEN(instr_gen, "LT");
                        float_counter++;
                        check_e();
                        if (expr_type == VT_FLOAT)
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                        }
                        int_counter++;
                        float_counter--;
                        INSTR_GEN(instr_gen, "EQ GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        INSTR_GEN_ADD_S(instr_gen, tmp_eq);
                        INSTR_GEN(instr_gen, "OR GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter-1);
                        free(tmp_eq);
                        e_type = ET_BOOLEAN;
                        float_counter++;
                        break;
                case TO_GREATER:
                        do_op("GT");
                        check_e();
                        if (expr_type == VT_FLOAT)
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                        }
                        e_type = ET_BOOLEAN;
                        break;
                case TO_GREATER_EQUAL:
                        expr_type_prev = expr_type;
                        accept(token.token_type);
                        tmp_s = malloc(strlen(string_get_string(&gen_buffer))+1);
                        strcpy(tmp_s, string_get_string(&gen_buffer));
                        string_terminate(&gen_buffer);
                        switch(expr_type_prev)
                        {
                        case VT_INT:
                                int_counter++;
                                break;
                        case VT_FLOAT:
                                float_counter++;
                                break;
                        case VT_STRING:
                                string_counter++;
                                break;
                        default:
                                break;
                        }
                        HANDLE_CALL_F(arith);
                        if (expr_type_prev == VT_INT && expr_type == VT_FLOAT)
                        {
                                float_counter++;
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter - 1);
                                string_terminate(&gen_buffer);
                                char * gen = malloc(100); *gen = 0;
                                sprintf(gen, " GF@E_FLOAT%d", float_counter);
                                strcpy(string_get_string(&gen_buffer), gen);
                                free(gen);
                                float_counter--;
                        }
                        else if (expr_type_prev == VT_FLOAT && expr_type == VT_INT)
                        {
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                string_terminate(&gen_buffer);
                                char * gen = malloc(100); *gen = 0;
                                sprintf(gen, " GF@E_FLOAT%d", float_counter - 1);
                                strcat(string_get_string(&gen_buffer), gen);
                                expr_type = ET_FLOAT;
                                free(gen);
                        }
                        else
                        {
                                strcpy(string_get_string(&gen_buffer), tmp_s);
                        }
                        t1 = malloc(100);
                        if (expr_type == VT_FLOAT || expr_type_prev == VT_FLOAT)
                        {
                                sprintf(t1, " GF@E_FLOAT%d", float_counter);
                                expr_type = VT_FLOAT;
                        }
                        else if (expr_type == VT_STRING && expr_type_prev == VT_STRING)
                        {
                                sprintf(t1, " GF@E_STRING%d", string_counter);
                                expr_type = VT_STRING;
                        }
                        else
                        {
                                sprintf(t1, " GF@E_INT%d", int_counter);
                                expr_type = VT_INT;
                        }
                        strcat(gen_buffer.pString, t1);
                        free(t1);
                        free(tmp_s);
                        tmp_eq = malloc(strlen(gen_buffer.pString) + 1);
                        strcpy(tmp_eq, gen_buffer.pString);
                        INSTR_GEN(instr_gen, "GT");
                        float_counter++;
                        check_e();
                        if (expr_type == VT_FLOAT)
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                        }
                        float_counter--;
                        int_counter++;
                        INSTR_GEN(instr_gen, "EQ GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        INSTR_GEN_ADD_S(instr_gen, tmp_eq);
                        INSTR_GEN(instr_gen, "OR GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter - 1);
                        free(tmp_eq);
                        e_type = ET_BOOLEAN;
                        float_counter++;
                        break;
                default:
                        break;
                }
        }
        return E_OK;
}

static int arith() {
        //    arith  : term_intd ((PLUS or MINUS) term_intd)*
        HANDLE_CALL_F(term_intd);

        while(isArith()) {
                switch(token.token_type) {
                case TO_PLUS:
                case TO_MINUS:
                        expr_type_prev = expr_type;
                        char * op_s;
                        if (token.token_type == TO_PLUS)
                        {
                                if (expr_type_prev == VT_STRING)
                                {
                                        op_s = "CONCAT"; concat = true;
                                }
                                else
                                {
                                        op_s = "ADD";
                                }
                        }
                        else
                        {
                                op_s = "SUB";
                        }
                        accept(token.token_type);
                        char * tmp_s = malloc(strlen(string_get_string(&gen_buffer))+1);
                        strcpy(tmp_s, string_get_string(&gen_buffer));
                        string_terminate(&gen_buffer);
                        switch(expr_type_prev)
                        {
                        case VT_INT:
                                int_counter++;
                                break;
                        case VT_FLOAT:
                                float_counter++;
                                break;
                        case VT_STRING:
                                string_counter++;
                        default:
                                break;
                        }
                        HANDLE_CALL_F(term_intd);
                        if (expr_type_prev == VT_INT && expr_type == VT_FLOAT)
                        {
                                float_counter++;
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter - 1);
                                string_terminate(&gen_buffer);
                                char * gen = malloc(100); *gen = 0;
                                sprintf(gen, " GF@E_FLOAT%d", float_counter);
                                strcpy(string_get_string(&gen_buffer), gen);
                                free(gen);
                                float_counter--;
                        }
                        else if (expr_type_prev == VT_FLOAT && expr_type == VT_INT)
                        {
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                string_terminate(&gen_buffer);
                                char * gen = malloc(100); *gen = 0;
                                sprintf(gen, " GF@E_FLOAT%d", float_counter - 1);
                                //strcpy(string_get_string(&gen_buffer), tmp_s);
                                strcat(string_get_string(&gen_buffer), gen);
                                expr_type = ET_FLOAT;
                                free(gen);
                        }
                        else
                        {
                                strcpy(string_get_string(&gen_buffer), tmp_s);
                        }
                        char * t1 = malloc(100);
                        switch(expr_type)
                        {
                        case ET_INT:
                                sprintf(t1, " GF@E_INT%d", int_counter);
                                break;
                        case ET_FLOAT:
                                sprintf(t1, " GF@E_FLOAT%d", float_counter);
                                break;
                        case ET_STRING:
                                sprintf(t1, " GF@E_STRING%d", string_counter);
                                break;
                        default:
                                break;
                        }
                        strcat(gen_buffer.pString, t1);
                        free(t1);
                        free(tmp_s);
                        INSTR_GEN(instr_gen, op_s);
                        check_e();
                        concat = false;
                        break;
                case T_EOL:
                default:
                        break;
                }
        }

        return E_OK;
}

static int term_intd() {

        //    term_intd : term (INT_DIV term)*

        HANDLE_CALL_F(term);

        while (isTerm_intd()) {
                switch(token.token_type) {
                case TO_BACKSLASH:
                        expr_type_prev = expr_type;
                        accept(TO_BACKSLASH);

                        if (expr_type_prev == VT_INT)
                        {
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                        }
                        else
                        {
                                return E_SEMANTIC_TYPE;
                        }
                        float_counter++;
                        HANDLE_CALL_F(term);
                        if (expr_type == VT_INT)
                        {
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                string_terminate(&gen_buffer);
                        }
                        else
                        {
                                return E_SEMANTIC_TYPE;
                        }
                        float_counter++;
                        INSTR_GEN(instr_gen, "DIV GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter-2);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter-1);
                        INSTR_GEN(instr_gen, "FLOAT2INT GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                        string_terminate(&gen_buffer);
                        break;
                default:
                        break;
                }
        }

        return E_OK;
}

static int term()
{
        // term : factor ((MUL or DIV) factor)*

        HANDLE_CALL_F(factor);

        while (isTerm()) {
                switch(token.token_type) {
                case TO_ASTERISK:
                        expr_type_prev = expr_type;
                        accept(token.token_type);
                        char * tmp_s = malloc(strlen(string_get_string(&gen_buffer))+1);
                        strcpy(tmp_s, string_get_string(&gen_buffer));
                        string_terminate(&gen_buffer);
                        switch(expr_type_prev)
                        {
                        case VT_INT:
                                int_counter++;
                                break;
                        case VT_FLOAT:
                                float_counter++;
                                break;
                        default:
                                return E_SEMANTIC_TYPE;
                                break;
                        }
                        HANDLE_CALL_F(term_intd);
                        if (expr_type_prev == VT_INT && expr_type == VT_FLOAT)
                        {
                                float_counter++;
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter - 1);
                                string_terminate(&gen_buffer);
                                char * gen = malloc(100); *gen = 0;
                                sprintf(gen, " GF@E_FLOAT%d", float_counter-1);
                                strcpy(string_get_string(&gen_buffer), gen);
                                free(gen);
                        }
                        else if (expr_type_prev == VT_FLOAT && expr_type == VT_INT)
                        {
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                string_terminate(&gen_buffer);
                                char * gen = malloc(100); *gen = 0;
                                sprintf(gen, " GF@E_FLOAT%d", float_counter - 1);
                                //strcpy(string_get_string(&gen_buffer), tmp_s);
                                strcat(string_get_string(&gen_buffer), gen);
                                expr_type = ET_FLOAT;
                                free(gen);
                        }
                        else
                        {
                                strcpy(string_get_string(&gen_buffer), tmp_s);
                        }
                        char * t1 = malloc(100);
                        switch(expr_type)
                        {
                        case ET_INT:
                                sprintf(t1, " GF@E_INT%d", int_counter);
                                break;
                        case ET_FLOAT:
                                sprintf(t1, " GF@E_FLOAT%d", float_counter);
                                break;
                        case ET_STRING:
                                sprintf(t1, " GF@E_STRING%d", string_counter);
                                break;
                        default:
                                break;
                        }
                        strcat(gen_buffer.pString, t1);
                        free(t1);
                        free(tmp_s);
                        INSTR_GEN(instr_gen, "MUL");
                        check_e();
                        break;
                case T_DIVISION:
                        expr_type_prev = expr_type;
                        accept(T_DIVISION);

                        if (expr_type_prev == VT_INT)
                        {
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                        }
                        else if (expr_type_prev != VT_FLOAT)
                        {
                                return E_SEMANTIC_TYPE;
                        }
                        float_counter++;
                        HANDLE_CALL_F(factor);
                        if (expr_type == VT_INT)
                        {
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                string_terminate(&gen_buffer);
                        }
                        else if (expr_type != VT_FLOAT)
                        {
                                return E_SEMANTIC_TYPE;
                        }
                        float_counter++;
                        INSTR_GEN(instr_gen, "DIV GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter-2);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter-1);
                        string_terminate(&gen_buffer);
                        e_type = ET_FLOAT;
                        break;
                case T_EOL: break;
                case TO_PLUS:
                case TO_MINUS:
                default:
                        break;
                }
        }
        return E_OK;
}


static int factor()
{
        // factor : INTEGER or (LBRACE E RBRACE) or FLOAT or Variable or string or function
        switch(token.token_type) {
        case T_INT:
                e_type = ET_INT;
                expr_type = VT_INT;
                accept(T_INT);
                string_append_string(&gen_buffer, " int@");
                char * tmp_str = malloc(100);
                *tmp_str = 0;
                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " int@");
                if (e_sign == S_MINUS)
                {
                        sprintf(tmp_str, "-%d", token.data_int);
                        INSTR_GEN_ADD_I(instr_gen, -1 * token.data_int);
                }
                else
                {
                        sprintf(tmp_str, "%d", token.data_int);
                        INSTR_GEN_ADD_I(instr_gen, token.data_int);
                }
                string_append_string(&gen_buffer, tmp_str);
                free(tmp_str);
                if (substr)
                {
                        int_counter++;
                        substr = false;
                }
                e_sign = S_NONE;
                break;
        case TD_OPEN_BRACE:
                accept(TD_OPEN_BRACE);
                int s = e_sign;
                HANDLE_CALL_F(E);
                accept(TD_CLOSE_BRACE);
                if (s == S_MINUS && e_type == ET_INT)
                {
                        INSTR_GEN(instr_gen, "MUL GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        INSTR_GEN_ADD_S(instr_gen, " int@-1");
                }
                if (s == S_MINUS && e_type == ET_FLOAT)
                {
                        INSTR_GEN(instr_gen, "MUL GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                        INSTR_GEN_ADD_S(instr_gen, " float@-1.0");
                }
                break;
        case T_FLOAT:
                e_type = ET_FLOAT;
                expr_type = VT_FLOAT;
                accept(T_FLOAT);

                string_append_string(&gen_buffer, " float@");
                tmp_str = malloc(100);
                *tmp_str = 0;
                INSTR_GEN(instr_gen, "MOVE GF@E_FLOAT");
                INSTR_GEN_ADD_I(instr_gen, float_counter);
                INSTR_GEN_ADD_S(instr_gen, " float@");
                if (e_sign == S_MINUS)
                {
                        sprintf(tmp_str, "-%g", token.data_float);
                        INSTR_GEN_ADD_F(instr_gen, -1 * token.data_float);
                }
                else
                {
                        sprintf(tmp_str, "%g", token.data_float);
                        INSTR_GEN_ADD_F(instr_gen, token.data_float);
                }
                string_append_string(&gen_buffer, tmp_str);
                free(tmp_str);
                e_sign = S_NONE;
                break;
        case T_ID:
                string_clear(&string);
                string_append_string(&string, token.data_string);
                accept(T_ID);

                if (token.token_type == TD_OPEN_BRACE)
                {
                        if (e_sign != S_NONE)
                        {
                                return E_SYNTACTIC;
                        }
                        htab * ht = DLGetFirst(htab_list);
                        htab_item *hti = get_htab_item(ht, string_get_string(&string));

                        if (hti == NULL || hti->var_func == VARIABLE)
                        {
                                ERROR_MSG("Parser: Wrong call %s\n", token.data_string);
                                return E_SEMANTIC_PRG;
                                // TODO: Which error it should be ?
                        }
                        else if (hti->var_func == FUNCTION)
                        {
                                if (!hti->f.declared && !hti->f.defined)
                                {
                                        ERROR_MSG("Parser: Called undeclared & undefined function %s\n", string_get_string(&string));
                                        return E_SEMANTIC_PRG;
                                }
                        }

                        stack_init(&params);
                        int ec = E_OK;
                        if ((ec = call()) != E_OK)
                        {
                                ERROR_MSG("Parser: Wrong function call %s\n", hti->name);
                                return ec;
                        }
                        ec = E_OK;
                        int tmp = vt;
                        vt = assign_type;
                        func_params = true;
                        if ((ec = check_func_signature(hti)) != E_OK)
                        {
                                ERROR_MSG("Parser: Wrong params or return type - function %s\n", hti->name);
                                return ec;
                        }
                        func_params = false;
                        vt = tmp;
                        stack_free(&params);
                        switch(hti->f.return_type)
                        {
                        case ET_INT:
                                e_type = ET_FUNC_INT;
                                break;
                        case ET_FLOAT:
                                e_type = ET_FUNC_FLOAT;
                                break;
                        case ET_STRING:
                                e_type = ET_FUNC_STRING;
                                break;
                        default:
                                break;
                        }

                        INSTR_GEN(instr_gen, "CALL $");
                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                }
                else
                {
                        htab_item *hti = DLGetFirstFound(htab_list, string_get_string(&string));

                        if (hti == NULL || hti->var_func == FUNCTION)
                        {
                                ERROR_MSG("Parser: Undefined ID %s or prev. defined as function\n", string_get_string(&string));
                                return E_SEMANTIC_PRG;
                        }

                        switch (hti->v.type)
                        {
                        case VT_INT:
                        {
                                e_type = ET_INT;
                                expr_type = VT_INT;
                                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                if (hti->v.scope == S_TEMP)
                                {
                                        INSTR_GEN_ADD_S(instr_gen, " GF@");
                                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                                        INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                                }
                                else
                                {
                                        INSTR_GEN_ADD_S(instr_gen, " LF@");
                                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                                }
                                if (e_sign == S_MINUS)
                                {
                                        INSTR_GEN(instr_gen, "MUL GF@E_INT");
                                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                                        INSTR_GEN_ADD_S(instr_gen, " int@-1");
                                }
                                if (substr)
                                {
                                        int_counter++;
                                        substr = false;
                                }
                                break;
                        }
                        case VT_FLOAT:
                        {
                                e_type = ET_FLOAT;
                                expr_type = VT_FLOAT;
                                INSTR_GEN(instr_gen, "MOVE GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                if (hti->v.scope == S_TEMP)
                                {
                                        INSTR_GEN_ADD_S(instr_gen, " GF@");
                                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                                        INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                                }
                                else
                                {
                                        INSTR_GEN_ADD_S(instr_gen, " LF@");
                                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                                }
                                if (e_sign == S_MINUS)
                                {
                                        INSTR_GEN(instr_gen, "MUL GF@E_FLOAT");
                                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                                        INSTR_GEN_ADD_S(instr_gen, " float@-1.0");
                                }
                                break;
                        }
                        case VT_STRING:
                        {
                                e_type = ET_STRING;
                                INSTR_GEN(instr_gen, "MOVE GF@E_STRING");
                                INSTR_GEN_ADD_I(instr_gen, string_counter);
                                if (hti->v.scope == S_TEMP)
                                {
                                        INSTR_GEN_ADD_S(instr_gen, " GF@");
                                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                                        INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                                }
                                else
                                {
                                        INSTR_GEN_ADD_S(instr_gen, " LF@");
                                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                                }
                                expr_type = VT_STRING;
                                break;
                        }
                        default:
                                ERROR_MSG("Parser: Expected type\n");
                                return E_SYNTACTIC;
                                break;
                        }

                        if (hti->v.scope == S_TEMP)
                        {
                                string_append_string(&gen_buffer, " GF@");
                                string_append_string(&gen_buffer, hti->name);
                                char * t = malloc(100);
                                *t = 0;
                                sprintf(t, "%d", hti->v.n);
                                string_append_string(&gen_buffer, t);
                                free(t);
                        }
                        else
                        {
                                string_append_string(&gen_buffer, " LF@");
                                string_append_string(&gen_buffer, hti->name);
                        }
                        e_sign = S_NONE;
                }
                break;
        case T_STRING:
                if (e_sign != S_NONE)
                {
                        return E_SYNTACTIC;
                }
                e_type = ET_STRING;
                INSTR_GEN(instr_gen, "MOVE GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter);
                INSTR_GEN_ADD_S(instr_gen, " string@");
                String *toFree = string_conv_spaces(token.data_string);
                INSTR_GEN_ADD_S(instr_gen, string_get_string(toFree));

                string_append_string(&gen_buffer, " string@");
                string_append_string(&gen_buffer, string_get_string(toFree));
                string_free(toFree);
                free(toFree);
                accept(T_STRING);
                expr_type = VT_STRING;
                break;
        case TO_MINUS:
                if (e_sign == S_MINUS)
                {
                        e_sign = S_PLUS;
                }
                else
                {
                        e_sign = S_MINUS;
                }
                accept(token.token_type);
                HANDLE_CALL_F(factor);
                break;
        case TO_PLUS:
                e_sign = S_PLUS;
                accept(token.token_type);
                HANDLE_CALL_F(factor);
                break;
        default:
                return E_SYNTACTIC; // TODO: check error
                break;
        }

        e_sign = S_NONE;
        return E_OK;
}


static int assign()
{
        if (token.token_type == TO_EQUAL) // = <E> rule
        {
                accept(TO_EQUAL);
                HANDLE_CALL_F(E);
                accept(T_EOL);
                return E_OK;
        }
        else if (token.token_type == T_EOL) // Epsylon rule
        {
                e_type = ET_EPSYLON;
                accept(T_EOL);
                return E_OK;
        }
        return E_SYNTACTIC;
}


static int statement_list()
{
        switch(token.token_type)
        {
        case TK_DIM:
        {
                HANDLE_CALL_F (declaration_list);
                break;
        }
        case T_EOL:
        {
                accept(T_EOL);
                break;
        }
        case T_ID:
        {
                string_clear(&string);
                string_append_string(&string, token.data_string);
                accept(T_ID);
                HANDLE_CALL_F (call_assign);
                break;
        }
        case TK_PRINT:
        {
                accept(TK_PRINT);
                HANDLE_CALL_F (print_list);
                break;
        }
        case TK_RETURN:
        {
                if (!in_func)
                {
                        ERROR_MSG("Parser: Return is outside of function.\n");
                        return E_SYNTACTIC;
                }
                accept(TK_RETURN);
                HANDLE_CALL_F(E);

                switch (e_type)
                {
                case ET_FLOAT:
                {
                        if (vt != VT_FLOAT && vt != VT_INT)
                        {
                                ERROR_MSG("Parser: Wrong return type.\n");
                                return E_SEMANTIC_TYPE;
                        }
                        else if (vt == VT_INT)
                        {
                                INSTR_GEN(instr_gen, "FLOAT2INT GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN(instr_gen, "MOVE LF@%retval GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                        }
                        else
                        {
                                INSTR_GEN(instr_gen, "MOVE LF@%retval GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                        }
                        break;
                }
                case ET_INT:
                {
                        if (vt != VT_FLOAT && vt != VT_INT)
                        {
                                ERROR_MSG("Parser: Wrong return type.\n");
                                return E_SEMANTIC_TYPE;
                        }

                        if (vt == VT_FLOAT)
                        {
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                INSTR_GEN(instr_gen, "MOVE LF@%retval GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                        }
                        else
                        {
                                INSTR_GEN(instr_gen, "MOVE LF@%retval GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                        }
                        break;
                }
                case ET_STRING:
                {
                        if (vt != VT_STRING)
                        {
                                ERROR_MSG("Parser: Wrong return type.\n");
                                return E_SEMANTIC_TYPE;
                        }
                        INSTR_GEN(instr_gen, "MOVE LF@%retval GF@E_STRING");
                        INSTR_GEN_ADD_I(instr_gen, string_counter);
                        break;
                }
                default:
                {
                        ERROR_MSG("Parser: Expected value to return\n");
                        return E_SEMANTIC_TYPE;
                        break;
                }
                }
                INSTR_GEN(instr_gen, "POPFRAME");
                INSTR_GEN(instr_gen, "RETURN");
                accept(T_EOL);
                break;
        }
        case TK_INPUT:
        {
                accept(TK_INPUT);
                if (token.token_type != T_ID)
                {
                        // ID excpected
                        return E_SYNTACTIC;
                }
                htab_item * hti = DLGetFirstFound(htab_list, token.data_string);
                if (hti == NULL || hti->var_func == FUNCTION)
                {
                        // ID not declared | declared as function
                        return E_SEMANTIC_PRG;
                        // TODO: Correct error type
                }

                INSTR_GEN(instr_gen, "MOVE GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter);
                INSTR_GEN_ADD_S(instr_gen, " string@?\\032");
                INSTR_GEN(instr_gen, "WRITE GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter);

                if (hti->v.scope == S_TEMP)
                {
                        INSTR_GEN(instr_gen, "READ GF@");
                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                        INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                }
                else
                {
                        INSTR_GEN(instr_gen, "READ LF@");
                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                }

                switch(hti->v.type)
                {
                case VT_INT:
                        INSTR_GEN_ADD_S(instr_gen, " int");
                        break;
                case VT_FLOAT:
                        INSTR_GEN_ADD_S(instr_gen, " float");
                        break;
                case VT_STRING:
                        INSTR_GEN_ADD_S(instr_gen, " string");
                        break;
                case VT_NONE:
                        return E_SEMANTIC_TYPE;
                        break;
                }

                accept(T_ID);
                accept(T_EOL);
                break;
        }
        case TK_IF:
        {
                int tmp_stmnt_counter = stmnt_counter;
                int else_if_counter = 0;
                stmnt_counter++;
                accept(TK_IF);
                HANDLE_CALL_F(E);

                if (e_type != ET_BOOLEAN)
                {
                        ERROR_MSG("Parser: Wrong if-else condition\n");
                        return E_SEMANTIC_TYPE;
                }

                INSTR_GEN(instr_gen, "JUMPIFEQ $ENDIF");
                INSTR_GEN_ADD_I(instr_gen, tmp_stmnt_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " bool@false");
                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " int@0");


                accept(TK_THEN);
                accept(T_EOL);
                ////DLAdd(htab_list);

                while (token.token_type != TK_ELSE && token.token_type != TK_END
                       && token.token_type != TK_ELSEIF && token.token_type != T_EOF)
                {
                        HANDLE_CALL_F (statement_list);
                }
                //DLDeleteLast(htab_list);

                INSTR_GEN(instr_gen, "JUMP $ENDELSE");
                INSTR_GEN_ADD_I(instr_gen, tmp_stmnt_counter);
                INSTR_GEN(instr_gen, "LABEL $ENDIF");
                INSTR_GEN_ADD_I(instr_gen, tmp_stmnt_counter);

                while (42)
                {
                        if (token.token_type == TK_END)
                        {
                                accept(TK_END);
                                accept(TK_IF);
                                accept(T_EOL);
                                INSTR_GEN(instr_gen, "LABEL $ENDELSE");
                                INSTR_GEN_ADD_I(instr_gen, tmp_stmnt_counter);
                                return E_OK;
                        }

                        if (token.token_type == TK_ELSEIF)
                        {
                                accept(TK_ELSEIF);
                                HANDLE_CALL_F(E);

                                if (e_type != ET_BOOLEAN)
                                {
                                        ERROR_MSG("Parser: Wrong if-else condition\n");
                                        return E_SEMANTIC_TYPE;
                                }

                                INSTR_GEN(instr_gen, "JUMPIFEQ $ELSEIF");
                                INSTR_GEN_ADD_I(instr_gen, tmp_stmnt_counter);
                                INSTR_GEN_ADD_I(instr_gen, else_if_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                INSTR_GEN_ADD_S(instr_gen, " bool@false");
                                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                INSTR_GEN_ADD_S(instr_gen, " int@0");


                                accept(TK_THEN);
                                accept(T_EOL);

                                while (token.token_type != TK_ELSE && token.token_type != TK_END
                                       && token.token_type != TK_ELSEIF && token.token_type != T_EOF)
                                {
                                        HANDLE_CALL_F (statement_list);
                                }

                                INSTR_GEN(instr_gen, "JUMP $ENDELSE");
                                INSTR_GEN_ADD_I(instr_gen, tmp_stmnt_counter);
                                INSTR_GEN(instr_gen, "LABEL $ELSEIF");
                                INSTR_GEN_ADD_I(instr_gen, tmp_stmnt_counter);
                                INSTR_GEN_ADD_I(instr_gen, else_if_counter);
                                else_if_counter++;
                        }
                        else
                        {
                                break;
                        }
                }

                accept(TK_ELSE);
                accept(T_EOL);
                //DLAdd(htab_list);

                while (token.token_type != TK_END && token.token_type != T_EOF)
                {
                        HANDLE_CALL_F (statement_list);
                }
                //DLDeleteLast(htab_list);
                accept(TK_END);
                accept(TK_IF);
                accept(T_EOL);
                INSTR_GEN(instr_gen, "LABEL $ENDELSE");
                INSTR_GEN_ADD_I(instr_gen, tmp_stmnt_counter);
                break;
        }
        case TK_DO:
        {
                int tmp_stmnt_counter = stmnt_counter;
                stmnt_counter++;
                accept(TK_DO);
                accept(TK_WHILE);
                INSTR_GEN(instr_gen, "LABEL $LOOP");
                INSTR_GEN_ADD_I(instr_gen, tmp_stmnt_counter);
                HANDLE_CALL_F(E);

                if (e_type != ET_BOOLEAN)
                {
                        ERROR_MSG("Parser: Wrong do-while condition\n");
                        return E_SEMANTIC_TYPE;
                }

                INSTR_GEN(instr_gen, "JUMPIFEQ $ENDLOOP");
                INSTR_GEN_ADD_I(instr_gen, tmp_stmnt_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " bool@false");
                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " int@0");
                accept(T_EOL);
                //DLAdd(htab_list);
                while (token.token_type != TK_LOOP && token.token_type != T_EOF)
                {
                        HANDLE_CALL_F (statement_list);
                }
                INSTR_GEN(instr_gen, "JUMP $LOOP");
                INSTR_GEN_ADD_I(instr_gen, tmp_stmnt_counter);
                INSTR_GEN(instr_gen, "LABEL $ENDLOOP");
                INSTR_GEN_ADD_I(instr_gen, tmp_stmnt_counter);
                //DLDeleteLast(htab_list);
                accept(TK_LOOP);
                accept(T_EOL);
                break;
        }
        default:
        {
                return E_SYNTACTIC;
        }
        }
        return E_OK;
}

static int print_list()
{
        HANDLE_CALL_F(E);
        switch (e_type)
        {
        case ET_INT:
        {
                INSTR_GEN(instr_gen, "WRITE GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                break;
        }
        case ET_FLOAT:
        {
                INSTR_GEN(instr_gen, "WRITE GF@E_FLOAT");
                INSTR_GEN_ADD_I(instr_gen, float_counter);
                break;
        }
        case ET_STRING:
        {
                INSTR_GEN(instr_gen, "WRITE GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter);
                break;
        }
        default:
        {
                return E_SEMANTIC_TYPE;
                break;
        }
        }
        accept(TD_SEMICOLON);
        if (token.token_type == T_EOL) // Epsylon rule
        {
                accept(T_EOL);
                return E_OK;
        }
        else // ; <E> rule
        {
                HANDLE_CALL_F (print_list);
                return E_OK;
        }
        return E_SYNTACTIC;
}


static int argument_list()
{
        static int argument_counter = 0;
        htab * ht = DLGetFirst(htab_list);
        htab_item * hti = get_htab_item(ht, string_get_string(&string));

        if (token.token_type == TD_CLOSE_BRACE) // Epsylon rule
        {
                return E_OK;
        }
        else if (argument_counter > hti->f.params.top)
        {
                ERROR_MSG("Parser: Too many arguments to function %s\n", hti->name);
                return E_SEMANTIC_TYPE;
        }

        HANDLE_CALL_F(E);

        switch (e_type)
        {
        case ET_INT:
        {
                stack_push(&params, VT_INT);
                if (func_params)
                {
                        if (hti->f.params.arr[argument_counter] == ET_FLOAT)
                        {
                                INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);

                                INSTR_GEN(instr_gen, "DEFVAR TF@");
                                INSTR_GEN_ADD_S(instr_gen, string_get_string(&hti->f.param_names[argument_counter]));

                                INSTR_GEN(instr_gen, "MOVE TF@");
                                INSTR_GEN_ADD_S(instr_gen, string_get_string(&hti->f.param_names[argument_counter++]));
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                        }
                        else if (hti->f.params.arr[argument_counter] == ET_INT)
                        {
                                INSTR_GEN(instr_gen, "DEFVAR TF@");
                                INSTR_GEN_ADD_S(instr_gen, string_get_string(&hti->f.param_names[argument_counter]));

                                INSTR_GEN(instr_gen, "MOVE TF@");
                                INSTR_GEN_ADD_S(instr_gen, string_get_string(&hti->f.param_names[argument_counter++]));
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                        }
                        else
                        {
                                ERROR_MSG("Wrong argument type\n");
                                return E_SEMANTIC_TYPE;
                        }
                }
                break;
        }
        case ET_FLOAT:
        {
                stack_push(&params, VT_FLOAT);
                if (func_params)
                {
                        if (hti->f.params.arr[argument_counter] == ET_INT)
                        {
                                INSTR_GEN(instr_gen, "FLOAT2INT GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);

                                INSTR_GEN(instr_gen, "DEFVAR TF@");
                                INSTR_GEN_ADD_S(instr_gen, string_get_string(&hti->f.param_names[argument_counter]));

                                INSTR_GEN(instr_gen, "MOVE TF@");
                                INSTR_GEN_ADD_S(instr_gen, string_get_string(&hti->f.param_names[argument_counter++]));
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                                INSTR_GEN_ADD_I(instr_gen, int_counter);
                        }
                        else if (hti->f.params.arr[argument_counter] == ET_FLOAT)
                        {
                                INSTR_GEN(instr_gen, "DEFVAR TF@");
                                INSTR_GEN_ADD_S(instr_gen, string_get_string(&hti->f.param_names[argument_counter]));

                                INSTR_GEN(instr_gen, "MOVE TF@");
                                INSTR_GEN_ADD_S(instr_gen, string_get_string(&hti->f.param_names[argument_counter++]));
                                INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                                INSTR_GEN_ADD_I(instr_gen, float_counter);
                        }
                        else
                        {
                                ERROR_MSG("Wrong argument type\n");
                                return E_SEMANTIC_TYPE;
                        }
                }
                else
                {
                        INSTR_GEN(instr_gen, "FLOAT2INT GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                        e_type = ET_INT;
                        if (substr)
                        {
                                int_counter++;
                                substr = false;
                        }
                }
                break;
        }
        case ET_STRING:
        {
                stack_push(&params, VT_STRING);
                if (func_params)
                {
                        if (hti->f.params.arr[argument_counter] != ET_STRING)
                        {
                                ERROR_MSG("Wrong argument type\n");
                                return E_SEMANTIC_TYPE;
                        }
                        INSTR_GEN(instr_gen, "DEFVAR TF@");
                        INSTR_GEN_ADD_S(instr_gen, string_get_string(&hti->f.param_names[argument_counter]));

                        INSTR_GEN(instr_gen, "MOVE TF@");
                        INSTR_GEN_ADD_S(instr_gen, string_get_string(&hti->f.param_names[argument_counter++]));
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING");
                        INSTR_GEN_ADD_I(instr_gen, string_counter);
                }
                break;
        }
        default:
        {
                ERROR_MSG("Parser: Expected param type\n");
                return E_SEMANTIC_TYPE; // TODO: Check return error correctness
        }
        }

        if (token.token_type == TD_COMMA) // , <E> rule
        {
                // Push Param to ADT
                accept(TD_COMMA);
                if (token.token_type == TD_CLOSE_BRACE)
                {
                        return E_SYNTACTIC;
                }
                //stack_clear(&params);
                HANDLE_CALL_F (argument_list);
                return E_OK;
        }
        else if (token.token_type == T_EOL) // Epsylon rule
        {
                accept(T_EOL);
                return E_OK;
        }
        if (token.token_type == TD_CLOSE_BRACE) // Epsylon rule
        {
                argument_counter = 0;
                return E_OK;
        }
        return E_SYNTACTIC;
}


static int check_func_signature(htab_item *hti)
{
        if (hti->f.return_type != vt)
        {
                if (hti->f.return_type == VT_STRING || vt == VT_STRING)
                {
                        // The return values doesnt fit
                        return E_SEMANTIC_TYPE;  // TODO : TYPE or PRG semantic error
                }
        }
        size_t params_count = stack_size(&hti->f.params);
        if (params_count != stack_size(&params))
        {
                // The count of params does not fit
                return E_SEMANTIC_TYPE; // TODO : TYPE or PRG semantic error
        }

        for (size_t i = 0; i < params_count; i++)
        {
                if (!func_params)
                {
                        if (hti->f.params.arr[i] != params.arr[i])
                        {
                                // The param type does not fit without implicit conversions
                                return E_SEMANTIC_PRG; // TODO : TYPE or PRG semantic error
                        }
                }
                else
                {
                        if ((hti->f.params.arr[i] == VT_STRING && params.arr[i] != VT_STRING)
                            || (hti->f.params.arr[i] != VT_STRING && params.arr[i] == VT_STRING))
                        {
                                // The param type does not fit, even with implicit conversions
                                return E_SEMANTIC_TYPE;  // TODO : TYPE or PRG semantic error
                        }
                }
        }
        return E_OK;
}

#define unary_op() do { \
                INSTR_GEN(instr_gen, op_s); \
                if (hti->v.scope == S_TEMP) \
                { \
                        INSTR_GEN_ADD_S(instr_gen, " GF@"); \
                        INSTR_GEN_ADD_S(instr_gen, hti->name); \
                        INSTR_GEN_ADD_I(instr_gen, hti->v.n); \
                        INSTR_GEN_ADD_S(instr_gen, " GF@"); \
                        INSTR_GEN_ADD_S(instr_gen, hti->name); \
                        INSTR_GEN_ADD_I(instr_gen, hti->v.n); \
                } \
                else \
                { \
                        INSTR_GEN_ADD_S(instr_gen, " LF@"); \
                        INSTR_GEN_ADD_S(instr_gen, hti->name); \
                        INSTR_GEN_ADD_S(instr_gen, " LF@"); \
                        INSTR_GEN_ADD_S(instr_gen, hti->name); \
                } \
} while(0)

static int call_assign()
{
        htab_item * hti;

        switch(token.token_type)
        {
        case TO_PLUS_EQUAL:
        case TO_MINUS_EQUAL:
        case TO_ASTERISK_EQUAL:
        case TO_SLASH_EQUAL:
        case TO_BACKSLASH_EQUAL:
        case TO_EQUAL:
                hti = DLGetFirstFound(htab_list, string_get_string(&string));
                if (hti == NULL)
                {
                        ERROR_MSG("Parser: Undefined variable %s\n", string_get_string(&string));
                        return E_SEMANTIC_PRG;
                }
                else if (hti->var_func == FUNCTION)
                {
                        return E_SEMANTIC_TYPE;
                }
                break;
        default:
                ERROR_MSG("Parser: Expected assignment '='.\n");
                return E_SYNTACTIC;
        }

        if (token.token_type == TO_EQUAL)
        {
                accept(TO_EQUAL);
                assign_type = hti->v.type;
                HANDLE_CALL_F (value);
        }
        else
        {
                int tmp_token = 0;
                char *op_s;
                if (hti->v.type == VT_STRING)
                {
                        if (token.token_type != TO_PLUS_EQUAL)
                        {
                                return E_SEMANTIC_TYPE;
                        }
                }

                switch (token.token_type)
                {
                case TO_PLUS_EQUAL:
                        if (hti->v.type == VT_STRING)
                        {
                                op_s = "CONCAT";

                        }
                        else
                        {
                                op_s = "ADD";
                        }
                        accept(TO_PLUS_EQUAL);
                        break;
                case TO_MINUS_EQUAL:
                        op_s = "SUB";
                        accept(TO_MINUS_EQUAL);
                        break;
                case TO_ASTERISK_EQUAL:
                        op_s = "MUL";
                        accept(TO_ASTERISK_EQUAL);
                        break;
                case TO_SLASH_EQUAL:
                        tmp_token = TO_SLASH_EQUAL;
                        op_s = "DIV";
                        accept(TO_SLASH_EQUAL);
                        break;
                case TO_BACKSLASH_EQUAL:
                        tmp_token = TO_BACKSLASH_EQUAL;
                        op_s = "DIV";
                        accept(TO_BACKSLASH_EQUAL);
                        break;
                default:
                        break;
                }
                HANDLE_CALL_F(E);
                if (tmp_token == TO_SLASH_EQUAL || tmp_token == TO_BACKSLASH_EQUAL)
                {
                        if (hti->v.type == VT_INT)
                        {
                                INSTR_GEN(instr_gen, "INT2FLOAT");
                                if (hti->v.scope == S_TEMP)
                                {
                                        INSTR_GEN_ADD_S(instr_gen, " GF@");
                                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                                        INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                                        INSTR_GEN_ADD_S(instr_gen, " GF@");
                                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                                        INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                                }
                                else
                                {
                                        INSTR_GEN_ADD_S(instr_gen, " LF@");
                                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                                        INSTR_GEN_ADD_S(instr_gen, " LF@");
                                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                                }
                                hti->v.type = VT_FLOAT;
                        }
                }
                if (hti->v.type == VT_INT && e_type == ET_INT)
                {
                        unary_op();
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                }
                else if (hti->v.type == VT_FLOAT && e_type == ET_FLOAT)
                {
                        unary_op();
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                }
                else if (hti->v.type == VT_STRING && e_type == ET_STRING)
                {
                        unary_op();
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING");
                        INSTR_GEN_ADD_I(instr_gen, string_counter);
                }
                else if (hti->v.type == VT_INT && e_type == ET_FLOAT)
                {
                        INSTR_GEN(instr_gen, "FLOAT2INT GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                        unary_op();
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                }
                else if (hti->v.type == VT_FLOAT && e_type == ET_INT)
                {
                        INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        unary_op();
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                }
                else
                {
                        return E_SEMANTIC_TYPE;
                }

                if (tmp_token == TO_BACKSLASH_EQUAL)
                {
                        INSTR_GEN(instr_gen, "FLOAT2INT");
                        if (hti->v.scope == S_TEMP)
                        {
                                INSTR_GEN_ADD_S(instr_gen, " GF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                                INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                                INSTR_GEN_ADD_S(instr_gen, " GF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                                INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                        }
                        else
                        {
                                INSTR_GEN_ADD_S(instr_gen, " LF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                                INSTR_GEN_ADD_S(instr_gen, " LF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                        }
                        hti->v.type = VT_INT;
                }
        }

        return E_OK;
}


#define EMBEDDED_F() do { \
                vt = hti->v.type; \
                tmp = func_params; \
                accept(token_type); \
                accept(TD_OPEN_BRACE); \
                func_params = false; \
                stack_init(&params); \
                HANDLE_CALL_F(argument_list); \
                htab * ht = DLGetFirst(htab_list); \
                htab_item * hti_f = get_htab_item(ht, htab_i_name); \
                func_params = true; \
                if ((ec = check_func_signature(hti_f)) != E_OK) \
                { \
                        return ec; \
                } \
                func_params = tmp; \
                stack_free(&params); \
                accept(TD_CLOSE_BRACE); \
} while (0)

static int value()
{
        bool tmp;
        int ec;

        htab_item *hti = DLGetFirstFound(htab_list, string_get_string(&string));

        if (hti == NULL || hti->var_func == FUNCTION)
        {
                ERROR_MSG("Parser: Wrong assignment %s\n", string_get_string(&string));
                return E_SEMANTIC_PRG;
                // TODO: Which error it should be ?
        }

        char * htab_i_name;
        int token_type;

        switch (token.token_type)
        {
        case TK_LENGTH:
                htab_i_name = "LENGTH";
                token_type = TK_LENGTH;
                EMBEDDED_F();
                INSTR_GEN(instr_gen, "STRLEN GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter);
                e_type = ET_INT;
                break;
        case TK_CHR:
                htab_i_name = "CHR";
                token_type = TK_CHR;
                EMBEDDED_F();
                INSTR_GEN(instr_gen, "INT2CHAR GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                e_type = ET_STRING;
                break;
        case TK_SUBSTR:
                substr = true;
                static int substr_counter = 0;
                htab_i_name = "SUBSTR";
                token_type = TK_SUBSTR;
                EMBEDDED_F();
                // Clear string
                INSTR_GEN(instr_gen, "MOVE GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter + 2);
                INSTR_GEN_ADD_S(instr_gen, " string@");
                // Is string empty condition
                INSTR_GEN(instr_gen, "STRLEN GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter);
                INSTR_GEN(instr_gen, "JUMPIFEQ $SUBSTR$END");
                INSTR_GEN_ADD_I(instr_gen, substr_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " int@0");
                // Is 'i' <= 0 condition
                INSTR_GEN(instr_gen, "LT GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter - 1);
                INSTR_GEN_ADD_S(instr_gen, " int@1");
                INSTR_GEN(instr_gen, "JUMPIFEQ $SUBSTR$END");
                INSTR_GEN_ADD_I(instr_gen, substr_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " bool@true");
                // n < 0 condition
                INSTR_GEN(instr_gen, "LT GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " int@0");
                INSTR_GEN(instr_gen, "JUMPIFEQ $SUBSTR$ELSE");
                INSTR_GEN_ADD_I(instr_gen, substr_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " bool@false");
                INSTR_GEN(instr_gen, "STRLEN GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter);
                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN(instr_gen, "JUMP $SUBSTR$ADD");
                INSTR_GEN_ADD_I(instr_gen, substr_counter);
                INSTR_GEN(instr_gen, "LABEL $SUBSTR$ELSE");
                INSTR_GEN_ADD_I(instr_gen, substr_counter);
                // n > Length(s)−i condition
                INSTR_GEN(instr_gen, "STRLEN GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter);
                INSTR_GEN(instr_gen, "SUB GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter - 1);
                INSTR_GEN(instr_gen, "GT GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN(instr_gen, "JUMPIFEQ $SUBSTR$ELSEELSE");
                INSTR_GEN_ADD_I(instr_gen, substr_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " bool@false");
                INSTR_GEN(instr_gen, "STRLEN GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter);
                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN(instr_gen, "JUMP $SUBSTR$ADD");
                INSTR_GEN_ADD_I(instr_gen, substr_counter);
                INSTR_GEN(instr_gen, "LABEL $SUBSTR$ELSEELSE");
                INSTR_GEN_ADD_I(instr_gen, substr_counter);
                // ADD
                INSTR_GEN(instr_gen, "ADD GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter - 1);
                INSTR_GEN(instr_gen, "SUB GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " int@1");
                INSTR_GEN(instr_gen, "LABEL $SUBSTR$ADD");
                INSTR_GEN_ADD_I(instr_gen, substr_counter);
                // Decrease positions, cuz we index from 1 not 0
                INSTR_GEN(instr_gen, "SUB GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter - 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter - 1);
                INSTR_GEN_ADD_S(instr_gen, " int@1");
                INSTR_GEN(instr_gen, "SUB GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " int@1");
                // Condition
                INSTR_GEN(instr_gen, "LABEL $SUBSTR$BEG");
                INSTR_GEN_ADD_I(instr_gen, substr_counter);
                INSTR_GEN(instr_gen, "GT GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter - 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN(instr_gen, "JUMPIFEQ $SUBSTR$END");
                INSTR_GEN_ADD_I(instr_gen, substr_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " bool@true");
                // Create string
                INSTR_GEN(instr_gen, "GETCHAR GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter - 1);
                // Append string
                INSTR_GEN(instr_gen, "CONCAT GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter + 2);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter + 2);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter + 1);
                // Increment
                INSTR_GEN(instr_gen, "ADD GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter - 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter - 1);
                INSTR_GEN_ADD_S(instr_gen, " int@1");
                // Jump back
                INSTR_GEN(instr_gen, "JUMP $SUBSTR$BEG");
                INSTR_GEN_ADD_I(instr_gen, substr_counter);
                // End label
                INSTR_GEN(instr_gen, "LABEL $SUBSTR$END");
                INSTR_GEN_ADD_I(instr_gen, substr_counter);

                string_counter += 2;
                int_counter += 1;
                substr_counter += 1;
                e_type = ET_STRING;
                break;
        case TK_ASC:
                htab_i_name = "ASC";
                static int asc_counter = 0;
                token_type = TK_ASC;
                EMBEDDED_F();
                // Get length of string
                INSTR_GEN(instr_gen, "STRLEN GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter);
                // We are indexing from 1
                INSTR_GEN(instr_gen, "SUB GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " int@1");
                // If index is lower then 0 condition
                INSTR_GEN(instr_gen, "LT GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 2);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " int@0");
                INSTR_GEN(instr_gen, "JUMPIFEQ $ASC$END");
                INSTR_GEN_ADD_I(instr_gen, asc_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 2);
                INSTR_GEN_ADD_S(instr_gen, " bool@true");
                // If index is bigger than length of string condition
                INSTR_GEN(instr_gen, "SUB GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN_ADD_S(instr_gen, " int@1");
                INSTR_GEN(instr_gen, "GT GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 2);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 1);
                INSTR_GEN(instr_gen, "JUMPIFEQ $ASC$END");
                INSTR_GEN_ADD_I(instr_gen, asc_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 2);
                INSTR_GEN_ADD_S(instr_gen, " bool@true");
                // Elsewhere index is OK
                INSTR_GEN(instr_gen, "STRI2INT GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 2);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter);
                INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter);
                INSTR_GEN(instr_gen, "JUMP $ASC$ENDEND");
                INSTR_GEN_ADD_I(instr_gen, asc_counter);
                INSTR_GEN(instr_gen, "LABEL $ASC$END");
                INSTR_GEN_ADD_I(instr_gen, asc_counter);
                INSTR_GEN(instr_gen, "MOVE GF@E_INT");
                INSTR_GEN_ADD_I(instr_gen, int_counter + 2);
                INSTR_GEN_ADD_S(instr_gen, " int@0");
                INSTR_GEN(instr_gen, "LABEL $ASC$ENDEND");
                INSTR_GEN_ADD_I(instr_gen, asc_counter);

                asc_counter += 1;
                int_counter += 2;
                e_type = ET_INT;
                break;
        default:
                HANDLE_CALL_F(E);
                break;
        }

        switch (e_type)
        {
        case ET_STRING:
        {
                if (hti->v.type != VT_STRING)
                {
                        ERROR_MSG("Parser: Wrong assignment %s\n", hti->name);
                        return E_SEMANTIC_TYPE;
                }

                if (hti->v.scope == S_TEMP)
                {
                        INSTR_GEN(instr_gen, "MOVE GF@");
                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                        INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                }
                else
                {
                        INSTR_GEN(instr_gen, "MOVE LF@");
                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                }
                INSTR_GEN_ADD_S(instr_gen, " GF@E_STRING");
                INSTR_GEN_ADD_I(instr_gen, string_counter);
                break;
        }
        case ET_FLOAT:
        {
                if (hti->v.type != VT_FLOAT && hti->v.type != VT_INT)
                {
                        ERROR_MSG("Parser: Wrong assignment %s\n", hti->name);
                        return E_SEMANTIC_TYPE;
                }
                else if (hti->v.type == VT_INT)
                {
                        INSTR_GEN(instr_gen, "FLOAT2INT GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                        if (hti->v.scope == S_TEMP)
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                                INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                        }
                        else
                        {
                                INSTR_GEN(instr_gen, "MOVE LF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                        }

                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                }
                else
                {
                        if (hti->v.scope == S_TEMP)
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                                INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                        }
                        else
                        {
                                INSTR_GEN(instr_gen, "MOVE LF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                        }

                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                }
                break;
        }
        case ET_INT:
        {
                if (hti->v.type != VT_FLOAT && hti->v.type != VT_INT)
                {
                        ERROR_MSG("Parser: Wrong assignment %s\n", hti->name);
                        return E_SEMANTIC_TYPE;
                }

                if (hti->v.type == VT_FLOAT)
                {
                        INSTR_GEN(instr_gen, "INT2FLOAT GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                        if (hti->v.scope == S_TEMP)
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                                INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                        }
                        else
                        {
                                INSTR_GEN(instr_gen, "MOVE LF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                        }
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_FLOAT");
                        INSTR_GEN_ADD_I(instr_gen, float_counter);
                }
                else
                {
                        if (hti->v.scope == S_TEMP)
                        {
                                INSTR_GEN(instr_gen, "MOVE GF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                                INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                        }
                        else
                        {
                                INSTR_GEN(instr_gen, "MOVE LF@");
                                INSTR_GEN_ADD_S(instr_gen, hti->name);
                        }
                        INSTR_GEN_ADD_S(instr_gen, " GF@E_INT");
                        INSTR_GEN_ADD_I(instr_gen, int_counter);
                }
                break;
        }
        case ET_FUNC_INT:
        case ET_FUNC_FLOAT:
        case ET_FUNC_STRING:
        {
                switch (e_type)
                {
                case ET_FUNC_INT:
                        if (hti->v.type == VT_INT)
                        {
                                break;
                        }
                        else if (hti->v.type == VT_FLOAT)
                        {
                                INSTR_GEN(instr_gen, "INT2FLOAT TF@%retval TF@%retval"); // TODO: Is it defined behavior ?
                        }
                        else
                        {
                                return E_SEMANTIC_TYPE;
                        }
                        break;
                case ET_FUNC_FLOAT:
                        if (hti->v.type == VT_INT)
                        {
                                INSTR_GEN(instr_gen, "FLOAT2INT TF@%retval TF@%retval"); // TODO: Is it defined behavior ?
                        }
                        else if (hti->v.type == VT_FLOAT)
                        {
                                break;
                        }
                        else
                        {
                                return E_SEMANTIC_TYPE;
                        }
                        break;
                case ET_FUNC_STRING:
                        if (hti->v.type == VT_STRING)
                        {
                                break;
                        }
                        else
                        {
                                return E_SEMANTIC_TYPE;
                        }
                        break;
                default:
                        return E_SEMANTIC_TYPE;
                        break;
                }
                if (hti->v.scope == S_TEMP)
                {
                        INSTR_GEN(instr_gen, "MOVE GF@");
                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                        INSTR_GEN_ADD_I(instr_gen, hti->v.n);
                }
                else
                {
                        INSTR_GEN(instr_gen, "MOVE LF@");
                        INSTR_GEN_ADD_S(instr_gen, hti->name);
                }
                INSTR_GEN_ADD_S(instr_gen, " TF@%retval");
                INSTR_GEN(instr_gen, "POPFRAME");
                return E_OK;
        }
        default:
        {
                ERROR_MSG("Parser: Wrong assignment %s\n", hti->name);
                return E_SEMANTIC_TYPE;
        }
        }
        accept(T_EOL);
        return E_OK;
}


static int call()
{
        bool tmp = func_params;
        accept(TD_OPEN_BRACE);
        stack_clear(&params);
        func_params = true;
        INSTR_GEN(instr_gen, "PUSHFRAME");
        INSTR_GEN(instr_gen, "CREATEFRAME");
        HANDLE_CALL_F (argument_list);
        func_params = tmp;
        accept(TD_CLOSE_BRACE);
        accept(T_EOL);
        return E_OK;
}

VAR_TYPE check_e_types(VAR_TYPE first, VAR_TYPE second)
{
        switch(first)
        {
        case VT_INT:
                if (second != VT_INT && second != VT_FLOAT)
                {
                        return VT_NONE;
                }
                return second;

        case VT_FLOAT:
                if (second != VT_INT && second != VT_FLOAT)
                {
                        return VT_NONE;
                }
                return VT_FLOAT;

        case VT_STRING:
                if (second != VT_STRING)
                {
                        return VT_NONE;
                }
                return VT_STRING;

        default:
                return VT_NONE;
        }
        return VT_NONE;
}

/**
 * returns string in IFJcode17 format
 */
static String * string_conv_spaces(char * str)
{
        String * s = malloc(sizeof *s); string_init(s);
        char currentChar;

        while (*str)
        {
                currentChar = *str++;
                if (isspace(currentChar))
                {
                        char escapeSeq[5]; *escapeSeq = 0;
                        sprintf(escapeSeq,"\\%03d", currentChar);
                        escapeSeq[4] = 0;
                        string_append_string(s, escapeSeq);
                }
                else
                {
                        string_push_char(s, currentChar);
                }
        }

        return s;
}

int parser()
{
        string_init(&gen_buffer);

        GET_TOKEN; if (token.token_type == T_EOF) return E_SYNTACTIC;
        string_init(&string);

        htab_list = malloc(sizeof *htab_list);
        DLInitList(htab_list);

        instr_gen = malloc(sizeof *instr_gen);
        INSTR_GEN_INIT(instr_gen);
        INSTR_GEN(instr_gen, ".IFJcode17");
        INSTR_GEN(instr_gen, "JUMP $$main");

        /// Create GF (global frame)
        DLAdd(htab_list);
        /// And push embedded funcs
        add_funcs_to_htab(DLGetFirst(htab_list));

        stack_init_string(&strstack);

        int rt = S();

        string_free(&string);
        string_free(&token_buffer);

        DLDisposeList(htab_list);
        free(htab_list);

        if (rt == E_OK)
        {
                INSTR_PRINT(instr_gen);
        }

        INSTR_GEN_FREE(instr_gen);
        free(instr_gen);

        stack_free_string(&strstack);
        string_free(&gen_buffer);
        return rt;
}