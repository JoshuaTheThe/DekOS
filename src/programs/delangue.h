#ifndef DELANGUE_H
#define DELANGUE_H

#include <stdint.h>
#include <stddef.h>

#include <utils.h>
#include <drivers/math.h>
#include <drivers/iso9660.h>

/* Keyboard, Mouse, TTY Output, that kind of thing */
#include <tty/output/output.h>
#include <tty/input/input.h>

#define STACK_SIZE (int)(4096)

/*

        / comment /
        square := function
                return arg['x'] + arg['x']
        end

        call add ['x':=2]

        // PUSH variable_by_name(add)->integer_value
        // CONSTRUCT_DICTIONARY (1)
        // PUSH 'x'
        // PUSH 2
        // SET_DICT
        // CALL (top)
*/

// arr :=* a, b, c, d

typedef enum
{
        DELANGUE_ERROR_NONE,
        DELANGUE_ERROR_SYNTAX,
        DELANGUE_ERROR_UNDEFINED_VARIABLE,
        DELANGUE_ERROR_TYPE_MISMATCH,
        DELANGUE_ERROR_STACK_OVERFLOW,
} delangueError_t;

typedef enum
{
        TOKEN_NULL,
        TOKEN_ASSIGN,
        TOKEN_ASSIGN_ARRAY, /* :=* */
        TOKEN_OPEN_BRACKET,
        TOKEN_CLOSE_BRACKET,
        TOKEN_COLON,
        TOKEN_COMMA,

        /* ADDITIVE */
        TOKEN_ADD,
        TOKEN_SUB,

        /* MULTIPLICATIVE */
        TOKEN_MUL,
        TOKEN_DIV,

        /* TERMS */
        TOKEN_STRING_LITERAL,
        TOKEN_INTEGER_LITERAL,
        TOKEN_IDENTIFIER,

        /* keyword */
        TOKEN_IF,
        TOKEN_WHILE,
        TOKEN_CALL,
        TOKEN_FUNCTION,
        TOKEN_END,
        TOKEN_RETURN,

} delangueTokenType_t;

typedef int delangueIntegerValue_t;
typedef char *delangueStringValue_t;
typedef int delangueProcedureValue_t;

typedef enum
{
        VALUE_NIL,
        VALUE_INTEGER,
        VALUE_STRING,
        VALUE_PROCEDURE,
        VALUE_ARRAY,
        VALUE_DICT,
} delangueValueType_t;

typedef struct __attribute__((__packed__))
{
        void *data;
        delangueValueType_t type;
} delangueValue_t;

typedef struct __attribute__((__packed__))
{
        size_t elements;
        delangueValue_t *values;
} delangueArrayValue_t;

typedef struct __attribute__((__packed__)) delangueDictEntry
{
        delangueStringValue_t key;
        delangueValue_t value;
        uint32_t hash;
        struct delangueDictEntry *next;
} delangueDictEntry_t;

typedef struct __attribute__((__packed__))
{
        size_t capacity;
        size_t count;
        delangueDictEntry_t **buckets;
} delangueDictionaryValue_t;

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
        char *identifier;
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

#endif
