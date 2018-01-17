/**
 * @file reserved.c
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

#include "reserved.h"

const char * reserved_keywords[] =
{
        "AS",
        "ASC",
        "DECLARE",
        "DIM",
        "DO",
        "DOUBLE",
        "ELSE",
        "END",
        "CHR",
        "FUNCTION",
        "IF",
        "INPUT",
        "INTEGER",
        "LENGTH",
        "LOOP",
        "PRINT",
        "RETURN",
        "SCOPE",
        "STRING",
        "SUBSTR",
        "THEN",
        "WHILE",
        // ---------
        "AND",
        "BOOLEAN",
        "CONTINUE",
        "ELSEIF",
        "EXIT",
        "FALSE",
        "FOR",
        "NEXT",
        "NOT",
        "OR",
        "SHARED",
        "STATIC",
        "TRUE"
};

const char operators[] =
{
        '*',
        '/',
        '\\',
        '<',
        '>',
        '=',
        '+',
        '-'
};

const char delimeters[] =
{
        '(',
        ')',
        ';',
        ','
};

bool is_reserved_keyword(String * s)
{
        unsigned int i;
        string_to_upper(s);

        for (i = 0; i < RESERVED_KEYWORDS_SIZE; i++)
        {
                if (!strcmp(string_get_string(s), reserved_keywords[i]))
                {
                        return true;
                }
        }
        return false;
}

KEYWORD_TYPE get_keyword_type(String * s)
{
        KEYWORD_TYPE kt = K_NONE;

        string_to_upper(s);
        char *tmp = string_get_string(s);

        switch(tmp[0])
        {
        case 'A':
                if (strcmp(reserved_keywords[K_AS], tmp) == 0)
                {
                        kt = K_AS;
                        break;
                }
                else if (strcmp(reserved_keywords[K_ASC], tmp) == 0)
                {
                        kt = K_ASC;
                        break;
                }
                else if (strcmp(reserved_keywords[K_AND], tmp) == 0)
                {
                        kt = K_AND;
                        break;
                }
                break;

        case 'B':
                if (strcmp(reserved_keywords[K_BOOLEAN], tmp) == 0)
                {
                        kt = K_BOOLEAN;
                        break;
                }
                break;

        case 'C':
                if (strcmp(reserved_keywords[K_CHR], tmp) == 0)
                {
                        kt = K_CHR;
                        break;
                }
                else if (strcmp(reserved_keywords[K_CONTINUE], tmp) == 0)
                {
                        kt = K_CONTINUE;
                        break;
                }
                break;

        case 'D':
                if (strcmp(reserved_keywords[K_DECLARE], tmp) == 0)
                {
                        kt = K_DECLARE;
                        break;
                }
                else if (strcmp(reserved_keywords[K_DO], tmp) == 0)
                {
                        kt = K_DO;
                        break;
                }
                else if (strcmp(reserved_keywords[K_DIM], tmp) == 0)
                {
                        kt = K_DIM;
                        break;
                }
                else if (strcmp(reserved_keywords[K_DOUBLE], tmp) == 0)
                {
                        kt = K_DOUBLE;
                        break;
                }
                break;

        case 'E':
                if (strcmp(reserved_keywords[K_END], tmp) == 0)
                {
                        kt = K_END;
                        break;
                }
                else if (strcmp(reserved_keywords[K_EXIT], tmp) == 0)
                {
                        kt = K_EXIT;
                        break;
                }
                else if (strcmp(reserved_keywords[K_ELSE], tmp) == 0)
                {
                        kt = K_ELSE;
                        break;
                }
                else if (strcmp(reserved_keywords[K_ELSEIF], tmp) == 0)
                {
                        kt = K_ELSEIF;
                        break;
                }
                break;

        case 'F':
                if (strcmp(reserved_keywords[K_FOR], tmp) == 0)
                {
                        kt = K_FOR;
                        break;
                }
                else if (strcmp(reserved_keywords[K_FALSE], tmp) == 0)
                {
                        kt = K_FALSE;
                        break;
                }
                else if (strcmp(reserved_keywords[K_FUNCTION], tmp) == 0)
                {
                        kt = K_FUNCTION;
                        break;
                }
                break;

        case 'I':
                if (strcmp(reserved_keywords[K_IF], tmp) == 0)
                {
                        kt = K_IF;
                        break;
                }
                else if (strcmp(reserved_keywords[K_INPUT], tmp) == 0)
                {
                        kt = K_INPUT;
                        break;
                }
                else if (strcmp(reserved_keywords[K_INPUT], tmp) == 0)
                {
                        kt = K_INPUT;
                        break;
                }
                else if (strcmp(reserved_keywords[K_INTEGER], tmp) == 0)
                {
                        kt = K_INTEGER;
                        break;
                }
                break;

        case 'L':
                if (strcmp(reserved_keywords[K_LOOP], tmp) == 0)
                {
                        kt = K_LOOP;
                        break;
                }
                else if (strcmp(reserved_keywords[K_LENGTH], tmp) == 0)
                {
                        kt = K_LENGTH;
                        break;
                }
                break;

        case 'N':
                if (strcmp(reserved_keywords[K_NONE], tmp) == 0)
                {
                        kt = K_NONE;
                        break;
                }
                else if (strcmp(reserved_keywords[K_NOT], tmp) == 0)
                {
                        kt = K_NOT;
                        break;
                }
                else if (strcmp(reserved_keywords[K_NEXT], tmp) == 0)
                {
                        kt = K_NEXT;
                        break;
                }
                break;

        case 'O':
                if (strcmp(reserved_keywords[K_OR], tmp) == 0)
                {
                        kt = K_OR;
                        break;
                }
                break;

        case 'P':
                if (strcmp(reserved_keywords[K_PRINT], tmp) == 0)
                {
                        kt = K_PRINT;
                        break;
                }
                break;

        case 'R':
                if (strcmp(reserved_keywords[K_RETURN], tmp) == 0)
                {
                        kt = K_RETURN;
                        break;
                }
                break;

        case 'S':
                if (strcmp(reserved_keywords[K_SCOPE], tmp) == 0)
                {
                        kt = K_SCOPE;
                        break;
                }
                else if (strcmp(reserved_keywords[K_STATIC], tmp) == 0)
                {
                        kt = K_SCOPE;
                        break;
                }
                else if (strcmp(reserved_keywords[K_SHARED], tmp) == 0)
                {
                        kt = K_SHARED;
                        break;
                }
                else if (strcmp(reserved_keywords[K_SUBSTR], tmp) == 0)
                {
                        kt = K_SUBSTR;
                        break;
                }
                else if (strcmp(reserved_keywords[K_STRING], tmp) == 0)
                {
                        kt = K_STRING;
                        break;
                }
                break;

        case 'T':
                if (strcmp(reserved_keywords[K_TRUE], tmp) == 0)
                {
                        kt = K_TRUE;
                        break;
                }
                else if (strcmp(reserved_keywords[K_THEN], tmp) == 0)
                {
                        kt = K_THEN;
                        break;
                }
                break;

        case 'W':
                if (strcmp(reserved_keywords[K_WHILE], tmp) == 0)
                {
                        kt = K_WHILE;
                        break;
                }
                break;
        }

        return kt;
}

bool is_operator(char op)
{
        unsigned int i;

        for (i = 0; i < OPERATORS_SIZE; i++)
        {
                if (operators[i] == op)
                {
                        return true;
                }
        }
        return false;
}

OPERATOR_TYPE get_operator_type(char op)
{
        switch(op)
        {
        case '*':
                return O_ASTERISK;
                break;

        case '/':
                return O_SLASH;
                break;

        case '\\':
                return O_BACKSLASH;
                break;

        case '<':
                return O_LOWER;
                break;

        case '>':
                return O_GREATER;
                break;

        case '=':
                return O_EQUAL;
                break;

        case '+':
                return O_PLUS;
                break;

        case '-':
                return O_MINUS;
                break;

        default:
                return O_NONE;
        }
}

bool is_delimeter(char d)
{
        unsigned int i;

        for (i = 0; i < DELIMETERS_SIZE; i++)
        {
                if (delimeters[i] == d)
                {
                        return true;
                }
        }
        return false;
}

DELIMETER_TYPE get_delimeter_type(char d)
{
        switch(d)
        {
        case '(':
                return D_OPEN_BRACE;
                break;

        case ')':
                return D_CLOSE_BRACE;
                break;

        case ';':
                return D_SEMICOLON;
                break;

        case ',':
                return D_COMMA;
                break;

        default:
                return D_NONE;
        }
}
