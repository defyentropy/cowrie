#ifndef LEXER_H
#define LEXER_H

typedef enum token_type
{
    TOKEN_IN,
    TOKEN_OUT,
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_BG,
    TOKEN_SEMICOLON,
    TOKEN_END
} TokenType;

typedef struct token
{
    char *start;
    int length;
    TokenType type;
    
} Token;

typedef struct lexer
{
    char *start;
    char *current;
} Lexer;

Token *lex_line(char *cmd_buf);
void sanitise_cmds(Token *tokens);

#endif
