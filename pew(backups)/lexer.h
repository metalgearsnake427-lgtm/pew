#ifndef LEXER_H
#define LEXER_H

typedef enum
{
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,

    TOKEN_ASSIGN,

    TOKEN_LPAREN,
    TOKEN_RPAREN,

    TOKEN_EOF

} TokenType;

typedef struct
{
    TokenType type;
    char text[64];
} Token;

void tokenize(const char *src);

#endif