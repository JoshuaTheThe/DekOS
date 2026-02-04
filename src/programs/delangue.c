
#include <programs/delangue.h>
#include <tty/output.h>
#include <memory/alloc.h>
#include <memory/string.h>

static char keywords[][MAX_KEYWORD_SIZE] = {
    "VAR",
    "FOR",
    "WHILE",
    "IF",
    "DO",
    "END",
    "RETURN",
};

void delangueTokenIdentifier(delangueState_t *state)
{
        char character = 0;
        uint32_t i;
        if (!state)
        {
                return;
        }

        state->current_token.identifier_length = 0;
        state->current_token.type = TOKEN_IDENTIFIER;

        do
        {
                character = state->current_working_file.raw_source[state->current_working_file.current_offset++];
                state->current_token.identifier[state->current_token.identifier_length] = character;
                state->current_token.identifier_length += 1;
        } while (((character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z')) && state->current_token.identifier_length < IDENTIFIER_SIZE);
        state->current_token.identifier[state->current_token.identifier_length - 1] = 0;
        --state->current_working_file.current_offset;

        printf("IDENTIFIER: '%s'\n", state->current_token.identifier); // DEBUG

        for (i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i)
        {
                if (strncmp(state->current_token.identifier, keywords[i], MAX_KEYWORD_SIZE) == 0)
                {
                        printf("FOUND KEYWORD: %s -> token type %d\n", keywords[i], i + TOKEN_KEYWORDS); // DEBUG
                        state->current_token.type = i + TOKEN_KEYWORDS;
                        break;
                }
        }
}

void delangueTokenNumber(delangueState_t *state)
{
        if (!state)
                return;

        state->current_token.integer_value = 0;
        state->current_token.type = TOKEN_NUMBER;

        char character;
        do
        {
                character = state->current_working_file.raw_source[state->current_working_file.current_offset];
                if (character >= '0' && character <= '9')
                {
                        state->current_token.integer_value *= 10;
                        state->current_token.integer_value += character - '0';
                        state->current_working_file.current_offset++;
                }
                else
                {
                        break;
                }
        } while (state->current_working_file.current_offset < state->current_working_file.remaining);
}

void delangueSkipWhitespace(delangueState_t *state)
{
        if (!state || state->current_working_file.current_offset >= state->current_working_file.remaining)
        {
                return;
        }

        char x = state->current_working_file.raw_source[state->current_working_file.current_offset];

        // Skip all whitespace characters
        while (state->current_working_file.current_offset < state->current_working_file.remaining &&
               (x == ' ' || x == '\t' || x == '\n' || x == '\r'))
        {
                state->current_working_file.current_offset++;
                if (state->current_working_file.current_offset < state->current_working_file.remaining)
                {
                        x = state->current_working_file.raw_source[state->current_working_file.current_offset];
                }
        }
}

void delangueToken(delangueState_t *state)
{
        if (!state)
        {
                /* TODO - ERRORS */
                return;
        }

        memset(state->current_token.identifier, 0, IDENTIFIER_SIZE);
        state->current_token.identifier_length = 0;
        state->current_token.integer_value = 0;
        state->current_token.type = TOKEN_NULL;

        delangueSkipWhitespace(state);
        char first = state->current_working_file.raw_source[state->current_working_file.current_offset];
        if (first >= '0' && first <= '9')
        {
                delangueTokenNumber(state);
        }
        else if ((first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z'))
        {
                delangueTokenIdentifier(state);
        }
        else if (first == '+')
        {
                state->current_token.type = TOKEN_PLUS;
                state->current_working_file.current_offset++;
        }
        else if (first == '-')
        {
                state->current_token.type = TOKEN_MINUS;
                state->current_working_file.current_offset++;
        }
        else if (first == '*')
        {
                state->current_token.type = TOKEN_MULTIPLY;
                state->current_working_file.current_offset++;
        }
        else if (first == '/')
        {
                state->current_token.type = TOKEN_DIVIDE;
                state->current_working_file.current_offset++;
        }
        else if (first == '.')
        {
                state->current_token.type = TOKEN_DOT;
                state->current_working_file.current_offset++;
        }
        else if (first == '(')
        {
                state->current_token.type = TOKEN_PARENTHESES_L;
                state->current_working_file.current_offset++;
        }
        else if (first == ')')
        {
                state->current_token.type = TOKEN_PARENTHESES_R;
                state->current_working_file.current_offset++;
        }

        state->current_working_file.remaining = state->current_working_file.length - state->current_working_file.current_offset;
        printf("TOKEN TYPE: %d,%d\n", state->current_token.type, state->current_token.integer_value);
}

void delangueNext(delangueState_t *state)
{
        if (state->current_working_file.remaining > 0)
        {
                delangueToken(state);
        }
        else
        {
                state->current_token.type = TOKEN_NULL;
        }
}

bool delangueAccept(delangueState_t *state, delangueTokenType_t token)
{
        if (state->current_token.type != token)
        {
                return false;
        }

        delangueNext(state);
        return true;
}

bool delangueExpect(delangueState_t *state, delangueTokenType_t token)
{
        if (delangueAccept(state, token))
        {
                return true;
        }
        printf("SYNTAX ERROR: Expected token %d, got %d at offset %d\n",
               token, state->current_token.type, state->current_working_file.current_offset);
        return false;
}

void delanguePush(delangueState_t *state, delangueValue_t value)
{
        if (state->virtual_machine.stack_ptr >= STACK_SIZE)
        {
                printf("RUNTIME ERROR: stack overflow\n");
                return;
        }
        state->virtual_machine.stack[state->virtual_machine.stack_ptr++] = value;
}

delangueValue_t delanguePop(delangueState_t *state)
{
        if (state->virtual_machine.stack_ptr == 0)
        {
                printf("RUNTIME ERROR: stack underflow\n");
                return (delangueValue_t){0};
        }
        return state->virtual_machine.stack[--state->virtual_machine.stack_ptr];
}

void delangueTerm(delangueState_t *state)
{
        delangueToken_t save = state->current_token;
        if (delangueAccept(state, TOKEN_IDENTIFIER))
        {
                printf("PUSH %s\n", save.identifier);
        }
        else if (delangueAccept(state, TOKEN_NUMBER))
        {
                delangueValue_t value;
                value.data = save.integer_value;
                delanguePush(state, value);
                printf("%d\n", value.data);
        }
        else
        {
                printf("ERROR\n");
        }
}

void delanguePost(delangueState_t *state)
{
        delangueTerm(state);
        while (state->current_token.type == TOKEN_PARENTHESES_L)
        {
                delangueNext(state);
                delangueExpression(state);
                delangueExpect(state, TOKEN_PARENTHESES_R);
        }
}

void delangueUnary(delangueState_t *state)
{
        if (state->current_token.type == TOKEN_MINUS)
        {
                /* e.g. -1 */
                delangueToken_t op = state->current_token;
                (void)op;
                delangueNext(state);
                delangueUnary(state);

                /* make it negative */
                delangueValue_t value = delanguePop(state);

                /* TODO - type checking (no types yet) */
                value.data = -value.data;
                delanguePush(state, value);
                printf("-\n");
        }
        else
        {
                delanguePost(state);
        }
}

void delangueMultiplicative(delangueState_t *state)
{
        printf("MUL");
        delangueUnary(state);
        while (state->current_token.type == TOKEN_MULTIPLY || state->current_token.type == TOKEN_DIVIDE)
        {
                delangueToken_t operator= state->current_token;
                delangueNext(state);
                delangueUnary(state);
                printf("*\n");
                delangueValue_t rhs = delanguePop(state);
                delangueValue_t lhs = delanguePop(state);
                if (operator.type == TOKEN_MULTIPLY)
                        lhs.data *= rhs.data;
                else if (rhs.data == 0)
                        lhs.data = -1;
                else
                        lhs.data /= rhs.data;
                delanguePush(state, lhs);
        }
}

void delangueExpression(delangueState_t *state)
{
        delangueMultiplicative(state);
        while (state->current_token.type == TOKEN_PLUS || state->current_token.type == TOKEN_MINUS)
        {
                delangueToken_t operator= state->current_token;
                delangueNext(state);
                delangueMultiplicative(state);
                printf("+\n");
                delangueValue_t rhs = delanguePop(state);
                delangueValue_t lhs = delanguePop(state);
                if (operator.type == TOKEN_PLUS)
                        lhs.data += rhs.data;
                else
                        lhs.data -= rhs.data;
                delanguePush(state, lhs);
        }
}

void delangueStatement(delangueState_t *state)
{
        if (!state || !state->current_working_file.raw_source)
                return;

        printf("STATEMENT: current token type = %d\n", state->current_token.type); // DEBUG

        switch (state->current_token.type)
        {
        case TOKEN_DO:
                printf("Found DO block\n");
                delangueNext(state); // Move past DO

                // Parse all statements until END
                while (state->current_token.type != TOKEN_NULL &&
                       state->current_token.type != TOKEN_END)
                {
                        delangueStatement(state);
                }

                if (state->current_token.type == TOKEN_END)
                {
                        delangueNext(state); // Move past END
                        delangueExpect(state, TOKEN_DOT);
                }
                break;

        case TOKEN_END:
                // Just return - END is handled by the block that started it
                printf("Found END, returning\n");
                return;

        case TOKEN_VAR:
                // Handle variable declaration
                delangueNext(state);
                // ... variable parsing logic
                delangueExpect(state, TOKEN_DOT);
                break;

        default:
                printf("Parsing expression statement\n");
                delangueExpression(state);
                delangueExpect(state, TOKEN_DOT);
                break;
        }
}

void delangueParse(delangueState_t *state)
{
        // Get the first token
        delangueNext(state);

        while (state->current_token.type != TOKEN_NULL)
        {
                delangueStatement(state);
        }

        printf("Final STACK SIZE: %d\n", state->virtual_machine.stack_ptr);
        if (state->virtual_machine.stack_ptr != 0)
        {
                delangueValue_t val = delanguePop(state);
                printf("EXIT VALUE = %d\n", val.data);
        }
}
