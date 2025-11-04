
#include <programs/delangue.h>

void delangueToken(delangueState_t *state)
{
        state->current_token.identifier = NULL;
        state->current_token.identifier_length = 0;
        state->current_token.integer_value = 0;
        state->current_token.type = TOKEN_NULL;

        size_t *offset = &state->current_working_file.current_offset;
}

void delangueNext(delangueState_t *state)
{
        if (state->current_working_file.remaining > 0)
        {
                delangueToken(state);
        }
        else
        {
                state->current_token.type = NULL;
        }
}

void delangueStatement(delangueState_t *state)
{
        while (state->current_token.type != TOKEN_NULL)
        {
                delangueNext(state);
        }
}
