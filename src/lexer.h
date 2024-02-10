#ifndef LEXER_H_
#define LEXER_H_
#include <ctype.h>
#include <string.h>

#define DEFAULT_LEXOUTCOME_INITIAL_CAPACITY 128
#define MAXIMUM_TOKEN_TXT_SIZE 64

typedef enum {
    UNKNOWN,

    LIT_STRING,
    LIT_INT,
    LIT_FLOAT,

    ID_INVOCATION,

    ID_VAR,
    VAR_SYM,

    ID_ROUTINE,
    ROUTINE_SYM,

    // keywords
    ROUTINE_END,
    KW_DUP,

    OP_PLUS,
    OP_PRINT,

    _IOTA
} TokenType;

char *ttype_tostr(TokenType ttype)
{
    switch (ttype) {
        case UNKNOWN:
            return "UNKNOWN";
            break;
        case LIT_STRING:
            return "LIT_STRING";
            break;
        case LIT_INT:
            return "LIT_INT";
            break;
        case LIT_FLOAT:
            return "LIT_FLOAT";
            break;
        case ID_INVOCATION:
            return "ID_INVOCATION";
            break;
        case ID_ROUTINE:
            return "ID_ROUTINE";
            break;
        case ROUTINE_SYM:
            return "ROUTINE_SYM";
            break;
        case ROUTINE_END:
            return "ROUTINE_END";
            break;
        case ID_VAR:
            return "ID_VAR";
            break;
        case VAR_SYM:
            return "VAR_SYM";
            break;
        case KW_DUP:
            return "KW_DUP";
            break;
        case OP_PLUS:
            return "OP_PLUS";
            break;
        case OP_PRINT:
            return "OP_PRINT";
            break;
        default:
            assert(0 && "Unreachable");
            break;
    }
}

char **token_types;
#define BUILD_TOKEN_TYPES(tt) do {               \
    tt = calloc(_IOTA, sizeof(char*));           \
    for (int i = 0; i < _IOTA; ++i) {            \
        char *str = ttype_tostr((TokenType) i);  \
        tt[i] = malloc(strlen(str));             \
        strcpy(tt[i], str);                      \
    }                                            \
} while (0)

void token_types_log() {
    for (int i = 0; i < _IOTA; ++i) 
        printf("%s\n", token_types[i]);
}

typedef struct {
    size_t row;
    size_t col;
} Location;

typedef struct {
    char* txt;
    Location loc;
    TokenType ttype;
} Token;

typedef struct {
    Token **tokens;
    size_t count;
    size_t capacity;
    char *file_path;
} LexOutcome;

Token *tk_create(char *txt, Location loc, TokenType ttype)
{
    Token *token = malloc(sizeof(Token));
    if (token == NULL) {
        fprintf(stderr, ERR_PREFIX"Could not allocate memory\n", ERR_EXP);
        exit(EXIT_FAILURE);
    }

    // this function manages memory on its own
    token->txt = malloc(strlen(txt));
    if (token->txt == NULL) {
        fprintf(stderr, ERR_PREFIX"Could not allocate memory\n", ERR_EXP);
        exit(EXIT_FAILURE);
    }

    // copy whatever takes as parameter
    strcpy(token->txt, txt);

    token->loc = loc;
    token->ttype = ttype;
    return token;
}

void tk_log(Token *token)
{
    printf("%-15s:%zu:%-5zu %-15s\n", ttype_tostr(token->ttype), token->loc.row, token->loc.col, token->txt);
}

void tk_destroy(Token *token)
{
    free(token->txt);
    free(token);
}

LexOutcome *loutcome_create(char *file_path, const size_t initial_capacity)
{
    LexOutcome *lex_outcome = malloc(sizeof(LexOutcome));
    if (lex_outcome == NULL) {
        fprintf(stderr, ERR_PREFIX"Could not allocate memory\n", ERR_EXP);
        exit(EXIT_FAILURE);
    }

    lex_outcome->file_path = file_path;
    lex_outcome->count = 0;
    lex_outcome->capacity = initial_capacity;

    lex_outcome->tokens = calloc(initial_capacity, sizeof(*lex_outcome->tokens));
    if (lex_outcome->tokens == NULL) {
        fprintf(stderr, ERR_PREFIX"Could not allocate memory\n", ERR_EXP);
        exit(EXIT_FAILURE);
    }

    return lex_outcome;
}

void loutcome_append(LexOutcome *lex_outcome, Token *token)
{
    // reallocate memory doubling space
    if (lex_outcome->count == lex_outcome->capacity) {

        // new computed capacity
        lex_outcome->capacity = lex_outcome->capacity == 0 ? DEFAULT_LEXOUTCOME_INITIAL_CAPACITY : (lex_outcome->capacity*2);

        lex_outcome->tokens = realloc(lex_outcome->tokens, lex_outcome->capacity*sizeof(*lex_outcome->tokens));
        if (lex_outcome->tokens == NULL) {
            fprintf(stderr, ERR_PREFIX"Could not allocate memory\n", ERR_EXP);
            exit(EXIT_FAILURE);
        }
    }

    lex_outcome->tokens[lex_outcome->count++] = token;
}

Token *loutcome_top(LexOutcome *lex_outcome)
{
    return *(lex_outcome->tokens + lex_outcome->count-1);
}

void loutcome_log(LexOutcome *lex_outcome)
{
    for (size_t i = 0; i < lex_outcome->count; ++i) {
        tk_log(*(lex_outcome->tokens+i));
    }
}

void loutcome_destroy(LexOutcome *lex_outcome)
{
    // deallocate all tokens
    for (size_t i = 0; i < lex_outcome->count; ++i)
        tk_destroy(*(lex_outcome->tokens+i));

    free(lex_outcome->tokens);
    free(lex_outcome);
}


LexOutcome *lex_buffer(char* buffer)
{
    LexOutcome *lex_outcome = loutcome_create("unknown.pc", 32);
    size_t buffer_size = strlen(buffer);

    // current char position
    size_t c = 0;

    // location tracking
    size_t row, col;
    row = col = 1;

    while (c < buffer_size) {
        if (buffer[c] == '\n') {
            // find a new line
            row++;
            c++;
            col = 1;
        } else if (isspace(buffer[c])) {
            c++;
            col++;
        } else if (buffer[c] == ';') {
            // encountered a comment, find end of line
            c++;
            while (buffer[c] != '\n') c++;
        } else {

            // token has been found
            char* txt = calloc(MAXIMUM_TOKEN_TXT_SIZE, 1);
            TokenType ttype = UNKNOWN;

            // column on which token start
            size_t col_start = 0;

            // txt start position related to buffer
            size_t c_start = c;

            if (buffer[c] == '"') {

                // skip opening double quotes
                c++;
                col++;
                c_start++;

                col_start = col;
                // allow char escaping, find end of string literal
                while (buffer[c] != '"' || buffer[c-1] == '\\') {
                    c++;
                    col++;
                }

                strncat(txt, buffer + c_start, c - c_start);
                ttype = LIT_STRING;

                // skip closing double quotes
                c++;
                col++;

            } else if (isalpha(buffer[c])) {
                // find identifier or keyword

                col_start = col;
                // allow numbers and hyphen symbol after first char
                while (isalnum(buffer[c]) || buffer[c] == '-') {
                    c++;
                    col++;
                }

                strncat(txt, buffer + c_start, c - c_start);

                // determine token type
                if (lex_outcome->count != 0) {
                    // check if it is and identifier
                    TokenType tt = loutcome_top(lex_outcome)->ttype;
                    if (tt == ROUTINE_SYM) ttype = ID_ROUTINE;
                    else if (tt == VAR_SYM) ttype = ID_VAR;
                }

                // type stills unkown, so it's not an ID
                if (ttype == UNKNOWN) {
                    // check if it is a keyword
                    if (strcmp(txt, "end") == 0) ttype = ROUTINE_END;
                    else if (strcmp(txt, "dup") == 0) ttype = KW_DUP;

                        // identifier invacation
                    else ttype = ID_INVOCATION;
                }

            } else if (isdigit(buffer[c])) {
                // find numeric literal

                bool flt = false;
                col_start = col;
                while (isdigit(buffer[c])) {
                    c++;
                    col++;
                    if (buffer[c] == '.') {
                        flt = true;
                        c++;
                        col++;
                    }
                }

                strncat(txt, buffer + c_start, c - c_start);
                if (flt) ttype = LIT_FLOAT;
                else ttype = LIT_INT;

            } else {
                // find a symbol, or whatever doesn't match previous if statements
                col_start = col;
                strncat(txt, buffer + c_start, 1);
                if (strcmp(txt, "@") == 0) ttype = VAR_SYM;
                else if (strcmp(txt, ":") == 0) ttype = ROUTINE_SYM;
                else if (strcmp(txt, ".") == 0) ttype = OP_PRINT;
                else if (strcmp(txt, "+") == 0) ttype = OP_PLUS;

                c++;
                col++;
            }

            Token *tk = tk_create(txt, (Location) {row, col_start}, ttype);
            loutcome_append(lex_outcome, tk);
        }
    }

    return lex_outcome;
}

#endif  // LEXER_H_
