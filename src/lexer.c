#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/lexer.h"

// fast-forwards the parser through whitespace
static void skip_whitespace(Lexer *lexer)
{
    while (*(lexer->current) == ' ' || *(lexer->current) == '\t')
    {
        lexer->current = lexer->current + 1;;
    }
}

static int is_special_char(char c)
{
    if (
        c == ' ' || 
        c == '|' || 
        c == '>' || 
        c == '<' || 
        c == '&' || 
        c == '\0' ||
        c == ';'
    )
    {
        return 1;
    }

    return 0;
}

// lexes a single token from the cmd input buffer
static Token lex_token(Lexer *lexer)
{
    skip_whitespace(lexer);

    Token new_token;
    new_token.start = lexer->current;

    switch (*(lexer->current))
    {
        case '<':
            new_token.length = 1;
            new_token.type = TOKEN_IN;
            lexer->current++;
            break;

        case '>':
            new_token.length = 1;
            new_token.type = TOKEN_OUT;
            lexer->current++;
            break;

        case '|':
            new_token.length = 1;
            new_token.type = TOKEN_PIPE;
            lexer->current++;
            break;

        case '&':
            new_token.length = 1;
            new_token.type = TOKEN_BG;
            lexer->current++;
            break;

        case ';':
            new_token.length = 1;
            new_token.type = TOKEN_SEMICOLON;
            lexer->current++;
            break;

        default:
            {
                new_token.length = 0;
                new_token.type = TOKEN_WORD;
                while (!is_special_char(*(lexer->current)))
                {
                    lexer->current++;
                    new_token.length++;
                }
            }
    }

    return new_token;
}

// lexes a full line of command input
Token *lex_line(char *cmd_buf)
{
    Lexer lexer;
    lexer.current = cmd_buf;

    size_t token_buf_size = 32;
    Token *token_buf = calloc(token_buf_size, sizeof(Token));
    if (token_buf == NULL)
    {
        perror("cowrie: allocation error");
        exit(EXIT_FAILURE);
    }

    int token_count = 0;

    while (*lexer.current != '\0')
    {
        token_buf[token_count++] = lex_token(&lexer);

        if (token_count >= token_buf_size)
        {
            token_buf_size *= 2;
            token_buf = reallocarray(token_buf, token_buf_size, sizeof(Token));
            
            if (token_buf == NULL)
            {
                perror("cowrie: allocation error");
                exit(EXIT_FAILURE);
            }


        }
    }

    Token end_token = {NULL, 0, TOKEN_END};
    token_buf[token_count] = end_token;

    return token_buf;
}

// copy individual lexemes into a new array of null-terminated strings,
// so pointers to them can be passed into execvp when the time comes for
// command execution
void sanitise_cmds(Token *tokens)
{
    // calculate exactly how many bytes will be needed
    size_t san_cmd_buf_size = 0;
    Token *current = tokens;
    while (current->type != TOKEN_END)
    {
        san_cmd_buf_size += current->length + 1;
        current++;
    }

    char *san_cmd_buf = malloc(san_cmd_buf_size * sizeof(char));
    if (san_cmd_buf == NULL)
    {
        perror("cowrie: allocation error");
        exit(EXIT_FAILURE);
    }

    char *san_token_ptr = san_cmd_buf;

    // copy lexemes into new buffer and add a null-terminator to each one
    current = tokens;
    while (current->type != TOKEN_END)
    {
        strncpy(san_token_ptr, current->start, current->length);
        current->start = san_token_ptr;
        san_token_ptr += current->length + 1;
        *(san_token_ptr - 1) = '\0';
        current++;
    }
}
