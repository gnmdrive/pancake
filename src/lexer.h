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
    KW_END,
    KW_DUP,
    KW_DROP,
    KW_SWAP,
    KW_OVER,

    OP_SUM,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,

    OP_PRINT,
    OP_PRINT_LN,
    OP_PRINT_MEM,

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
        case ID_VAR:
            return "ID_VAR";
            break;
        case VAR_SYM:
            return "VAR_SYM";
            break;
        case KW_END:
            return "KW_END";
            break;
        case KW_DUP:
            return "KW_DUP";
            break;
        case KW_DROP:
            return "KW_DROP";
            break;
        case KW_SWAP:
            return "KW_SWAP";
            break;
        case KW_OVER:
            return "KW_OVER";
            break;
        case OP_SUM:
            return "OP_SUM";
            break;
        case OP_SUB:
            return "OP_SUB";
            break;
        case OP_MUL:
            return "OP_MUL";
            break;
        case OP_DIV:
            return "OP_DIV";
            break;
        case OP_MOD:
            return "OP_MOD";
            break;
        case OP_PRINT_LN:
            return "OP_PRINT_LN";
            break;
        case OP_PRINT:
            return "OP_PRINT";
            break;
        case OP_PRINT_MEM:
            return "OP_PRINT_MEM";
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
} LexWork;

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

LexWork *lwork_create(char *file_path, const size_t initial_capacity)
{
    LexWork *lex_work = malloc(sizeof(LexWork));
    if (lex_work == NULL) {
        fprintf(stderr, ERR_PREFIX"Could not allocate memory\n", ERR_EXP);
        exit(EXIT_FAILURE);
    }

    lex_work->file_path = file_path;
    lex_work->count = 0;
    lex_work->capacity = initial_capacity;

    lex_work->tokens = calloc(initial_capacity, sizeof(*lex_work->tokens));
    if (lex_work->tokens == NULL) {
        fprintf(stderr, ERR_PREFIX"Could not allocate memory\n", ERR_EXP);
        exit(EXIT_FAILURE);
    }

    return lex_work;
}

void lwork_append(LexWork *lex_work, Token *token)
{
    // reallocate memory doubling space
    if (lex_work->count == lex_work->capacity) {

        // new computed capacity
        lex_work->capacity = lex_work->capacity == 0 ? DEFAULT_LEXOUTCOME_INITIAL_CAPACITY : (lex_work->capacity*2);

        lex_work->tokens = realloc(lex_work->tokens, lex_work->capacity*sizeof(*lex_work->tokens));
        if (lex_work->tokens == NULL) {
            fprintf(stderr, ERR_PREFIX"Could not allocate memory\n", ERR_EXP);
            exit(EXIT_FAILURE);
        }
    }

    lex_work->tokens[lex_work->count++] = token;
}

Token *lwork_top(LexWork *lex_work)
{
    return *(lex_work->tokens + lex_work->count-1);
}

void lwork_log(LexWork *lex_work)
{
    for (size_t i = 0; i < lex_work->count; ++i) {
        tk_log(*(lex_work->tokens+i));
    }
}

void lwork_destroy(LexWork *lex_work)
{
    // deallocate all tokens
    for (size_t i = 0; i < lex_work->count; ++i)
        tk_destroy(*(lex_work->tokens+i));

    free(lex_work->tokens);
    free(lex_work);
}


LexWork *lex_buffer(char* buffer)
{
    LexWork *lex_work = lwork_create("unknown.pc", 32);
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
                if (lex_work->count != 0) {
                    // check if it is and identifier
                    TokenType tt = lwork_top(lex_work)->ttype;
                    if (tt == ROUTINE_SYM) ttype = ID_ROUTINE;
                    else if (tt == VAR_SYM) ttype = ID_VAR;
                }

                // type stills unkown, so it's not an ID
                if (ttype == UNKNOWN) {
                    // check if it is a keyword
                    if (strcmp(txt, "end") == 0) ttype = KW_END;
                    else if (strcmp(txt, "dup") == 0) ttype = KW_DUP;
                    else if (strcmp(txt, "drop") == 0) ttype = KW_DROP; 
                    else if (strcmp(txt, "swap") == 0) ttype = KW_SWAP; 
                    else if (strcmp(txt, "over") == 0) ttype = KW_OVER; 

                    // identifier invacation
                    else ttype = ID_INVOCATION;
                }

            } else if (isdigit(buffer[c])) {
                // find numeric literal

                // current number is negative
                if (buffer[c-1] == '-') strcat(txt, "-");

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
                else if (strcmp(txt, "+") == 0) ttype = OP_SUM;
                else if (strcmp(txt, "*") == 0) ttype = OP_MUL;
                else if (strcmp(txt, "/") == 0) ttype = OP_DIV;
                else if (strcmp(txt, "%") == 0) ttype = OP_MOD;

                else if (strcmp(txt, "-") == 0) {
                    if (!isdigit(buffer[c+1])) ttype = OP_SUB;
                }
                else if (strcmp(txt, ".") == 0) {

                    if (buffer[c+1] == '.') {
                        if (buffer[c+2] == '.') {
                            strncat(txt, buffer + c_start, 2);
                            ttype = OP_PRINT_MEM;
                            c += 2;
                        } else {
                            strncat(txt, buffer + c_start, 1);
                            ttype = OP_PRINT;
                            c++;
                        }
                    }
                    else ttype = OP_PRINT_LN;
                }

                else {
                    fprintf(stderr, ERR_PREFIX"Symbol not recognized: %s\n", ERR_EXP, txt);
                    exit(EXIT_FAILURE);
                }

                c++;
                col++;
            }


            // if type is unknow then current token should not be added to the outcome
            if (ttype != UNKNOWN) {
                Token *tk = tk_create(txt, (Location) {row, col_start}, ttype);
                lwork_append(lex_work, tk);
            }
        }
    }

    return lex_work;
}

#endif  // LEXER_H_
