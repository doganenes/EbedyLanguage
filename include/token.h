#include <stdlib.h>
#ifndef TOKEN_H
#define TOKEN_H

typedef struct TOKEN_STRUCT
{
    enum
    {
        TOKEN_ID,
        TOKEN_EQUALS,
        TOKEN_STRING,
        TOKEN_SEMI,
        TOKEN_LPAREN,
        TOKEN_RPAREN,
        TOKEN_RBRACE,
        TOKEN_LBRACE,
        TOKEN_COMMA,
        TOKEN_EOF
    } type;

    char *value;
} token_T;

#include "token.h"

token_T *init_token(int type, char *value)
{
    token_T *token = malloc(sizeof(token_T));
    token->type = type;
    token->value = value;
    return token;
}

#endif
