#ifndef DELANGUE_H
#define DELANGUE_H

#include <stdint.h>
#include <stddef.h>

#include <utils.h>
#include <drivers/math.h>
#include <drivers/iso9660.h>

/*

FOR I=0 , I < 100 , I = I + 1 DO
        print (I).
END.

*/

/* Keyboard, Mouse, TTY Output, that kind of thing */
#include <tty/output/output.h>
#include <tty/input/input.h>

#define STACK_SIZE (int)(4096)
#define IDENTIFIER_SIZE (int)(256)
#define MAX_KEYWORD_SIZE (int)(32)

typedef enum
{
        DELANGUE_ERROR_NONE,
        DELANGUE_ERROR_SYNTAX,
        DELANGUE_ERROR_UNDEFINED_VARIABLE,
        DELANGUE_ERROR_STACK_OVERFLOW,
} delangueError_t;

typedef enum
{
        TOKEN_NULL,
        TOKEN_IDENTIFIER,
        TOKEN_NUMBER,
        
        TOKEN_DOT,
        TOKEN_PARENTHESES_L,
        TOKEN_PARENTHESES_R,

        TOKEN_PLUS,
        TOKEN_MINUS,

        TOKEN_MULTIPLY,
        TOKEN_DIVIDE,

        TOKEN_KEYWORDS,
        TOKEN_VAR=TOKEN_KEYWORDS,
        TOKEN_FOR,
        TOKEN_WHILE,
        TOKEN_IF,
        TOKEN_DO,
        TOKEN_END,
        TOKEN_RETURN,
} delangueTokenType_t;

typedef struct __attribute__((__packed__))
{
        int data;
} delangueValue_t;

typedef struct __attribute__((__packed__))
{
        const char *raw_source;
        size_t length;
        size_t current_offset;
        size_t remaining;
} delangueSource_t;

typedef struct __attribute__((__packed__))
{
        size_t identifier_length;
        char identifier[IDENTIFIER_SIZE];
        int integer_value;
        delangueTokenType_t type;
} delangueToken_t;

typedef struct
{
        delangueValue_t stack[STACK_SIZE];
        size_t stack_ptr;

        struct
        {
                char **names;
                delangueValue_t *values;
                size_t count;
                size_t capacity;
        } variables;

        delangueError_t error;
        char error_msg[256];
} delangueVM_t;

typedef struct __attribute__((__packed__))
{
        delangueToken_t current_token;
        delangueSource_t current_working_file;
        delangueVM_t virtual_machine;
} delangueState_t;

/*

term()
{
        next();
        switch (TOKEN)
        {
        case INTEGER:
                push int_to_value(num);
        case STRING:
                push string_to_value(str);
        case IDENTIFIER:
                push variable_to_value(variables[name]);
        default:
                ERROR
}

factor()
{
        term();
        while (is_multiplicative())
        {
                term();
                multiply and dividng
        }
}

EXPR()
{
        factor();
        while (is_additive())
        {
                factor();
                do adding / subtracting
        }
}

statement()
{
        while (true)
        {
                switch (TOKEN)
                {
                case IF:
                        if ()
                                ;
                        break;
                default:
                        EXPR();
                        break;
                }
        }
}

*/

void delangueExpression(delangueState_t *state);
void delangueStatement(delangueState_t *state);
void delangueParse(delangueState_t *state);

#endif
