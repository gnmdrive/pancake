#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "stack.h"

#define CHECK_LAST_NUMBERS_TYPE(mem, message) do {              \
if (atoi(st_peek(mem, 0)) == 0 || atoi(st_peek(mem, 1)) == 0) { \
    fprintf(stderr, message);                                   \
    exit(EXIT_FAILURE);                                         \
}                                                               \
} while (0)

#define ROUTINES_INITIAL_CAPACITY 16
#define VARIABLES_INITIAL_CAPACITY 16
#define TOKENS_INITIAL_CAPACITY 64

void retrieve_last_numbers_float(float *x, float *y, Stack *mem)
{
    // peek last and second-last items from stack
    char *a = st_peek(mem, 0);
    char *b = st_peek(mem, 1);

    float x_res = atof(a);
    float y_res = atof(b);

    st_pop(mem);
    st_pop(mem);

    *x = x_res;
    *y = y_res;
}


typedef struct {
    char *id;
    char *txt;
} Variable;

typedef struct {
    Variable var[8];
    size_t var_count;
} Parameters;

typedef struct {
    char *id;
    Token **tokens;
    size_t tk_count;
    size_t tk_capacity;
    // Parameters *params;
} Routine;

typedef struct {
    Routine **routines;
    // Variable **variables;
    size_t rte_capacity;
    size_t rte_count;
    // size_t var_capacity;
    // size_t var_count;
} GScope;

int gscope_search_routine(GScope *gscope, char *id)
{
    int j = 0;
    while (j < (int) gscope->rte_count && strcmp(id, gscope->routines[j]->id) != 0) j++;
    return j;
}

Routine *rte_create(char *id, const size_t tokens_initial_capacity)
{
    Routine *routine = malloc(sizeof(Routine));
    routine->id = id;

    routine->tk_count = 0;
    routine->tk_capacity = tokens_initial_capacity;
    routine->tokens = calloc(tokens_initial_capacity, sizeof(*routine->tokens));

    return routine;
}

void rte_append_token(Routine *routine, Token *tk)
{
    if (routine->tk_count == routine->tk_capacity) {
        routine->tk_capacity = routine->tk_capacity == 0 ? ROUTINES_INITIAL_CAPACITY : (routine->tk_capacity*2);

        routine->tokens = realloc(routine->tokens, routine->tk_capacity*sizeof(*routine->tokens));
    }

    routine->tokens[routine->tk_count++] = tk;
}

void rte_execute(Routine *routine, Stack *mem, GScope *gscope)
{
    const size_t tk_count = routine->tk_count;

    for (size_t i = 0; i < tk_count; ++i) {
        Token *tk = routine->tokens[i];

        switch (tk->ttype) {
            case LIT_STRING: {

                // check if string contains new line characters
                size_t c = 0;
                size_t string_len = strlen(tk->txt);

                // stored strings treat \n as two different chars
                while (c < string_len) {
                    if (tk->txt[c] == '\\' && tk->txt[c+1] == 'n') {
                        // can't use new line escape char inside string
                        fprintf(stderr, "ERROR: can't use new line escape char inside string\n");
                        exit(EXIT_FAILURE);
                    }
                    c++;
                }

                // everything is ok
                st_push(mem, tk->txt);

            } break;

            case LIT_FLOAT:
            case LIT_INT: {

                // push wathever number types
                st_push(mem, tk->txt);

            } break;

            case OP_SUM: 
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_MOD: {

                // stack should contains at least two numbers
                assert(mem->count >= 2);

                // verify is last two items in mem are actually numbers
                CHECK_LAST_NUMBERS_TYPE(mem, "ERROR: tried to operate on values that are not numbers\n");

                float x, y;
                retrieve_last_numbers_float(&x, &y, mem);

                char result[16];
                float numeric_result = 0;

                switch (tk->ttype) {
                    case OP_SUM: numeric_result = x + y;
                        break;
                    case OP_SUB: numeric_result = x - y;
                        break;
                    case OP_MUL: numeric_result = x * y;
                        break;
                    case OP_DIV: {
                        assert(y != 0 && "Can't divide by zero");
                        numeric_result = x / y;
                    } break;
                    case OP_MOD: numeric_result = fmod(x, (int) y);
                        break;
                    default:
                        assert(0 && "Unreachable");
                        break;
                }

                // verify if result has a fractional part
                double fractpart, intpart;
                fractpart = modf(numeric_result, &intpart);

                // if result its essentially an int then convert it
                if (fractpart == 0) {
                    sprintf(result, "%d", (int) numeric_result);
                } else sprintf(result, "%f", numeric_result);

                // push result as a string
                st_push(mem, result);

            } break;

            case OP_PRINT: {
                assert(mem->count >= 1);
                printf("%s", st_peek(mem, 0));
                st_pop(mem);
            } break;

            case OP_PRINT_LN: {
                assert(mem->count >= 1);
                printf("%s\n", st_peek(mem, 0));
                st_pop(mem);
            } break;

            case OP_PRINT_MEM: {
                assert(mem->count >= 1);
                st_display(mem);
            } break;

            case KW_DUP: {
                assert(mem->count >= 1);
                st_push(mem, st_peek(mem, 0));
            } break;

            case KW_DROP: {
                assert(mem->count >= 1);
                st_pop(mem);
            } break;

            case KW_SWAP: {
                assert(mem->count >= 2);
                char last[64];
                char second_last[64];

                strcpy(last, st_peek(mem, 0));
                strcpy(second_last, st_peek(mem, 1));

                st_pop(mem);
                st_pop(mem);

                st_push(mem, last);
                st_push(mem, second_last);

            } break;

            case KW_OVER: {
                assert(mem->count >= 2);
                st_push(mem, st_peek(mem, 1));
            } break;

            case KW_END: {

                if (strcmp(routine->id, "main") == 0) {
                    // main routine stack should be empty at program end
                    assert(mem->count == 0);
                }

                // TODO: should do something
            } break;

            case ID_INVOCATION: {

                // search routine in routines list
                size_t j = gscope_search_routine(gscope, tk->txt);

                // routines has been declared
                if (j != gscope->rte_count) {

                    // execute recursively routines
                    rte_execute(gscope->routines[j], mem, gscope);

                } else {
                    fprintf(stderr, ERR_PREFIX"Routine has not been declared: '%s'\n", ERR_EXP, tk->txt);
                    exit(EXIT_FAILURE);
                }

                // assert(0 && "Routines not implemented");

            } break;

            case ID_ROUTINE: {

                assert(0 && "Can't define routine inside routines");

            } break;

            case ID_VAR: {

                assert(0 && "Can't define var inside routines");

            } break;

            default: {
                fprintf(stderr, ERR_PREFIX"Can't interpret this token: '%s'\n", ERR_EXP, tk->txt);
                assert(0);
                // exit(EXIT_FAILURE);
            } break;
        }
    }
}

void rte_destroy(Routine *routine)
{
    for (size_t i = 0; i < routine->tk_count; ++i) {
        tk_destroy(routine->tokens[i]);
    }

    free(routine->tokens);
    free(routine);
}

GScope *gscope_create(const size_t rte_initial_capacity, const size_t var_initial_capacity)
{
    GScope *gscope = malloc(sizeof(GScope));

    gscope->rte_count = 0;
    // gscope->var_count = 0;

    gscope->rte_capacity = rte_initial_capacity;
    // gscope->var_capacity = var_initial_capacity;

    gscope->routines = calloc(rte_initial_capacity, sizeof(*gscope->routines));
    // gscope->variables = calloc(rte_initial_capacity, sizeof(*gscope->variables));

    return gscope;
}

void gscope_append_routine(GScope *gscope, Routine *routine)
{
    if (gscope->rte_count == gscope->rte_capacity) {
        gscope->rte_capacity = gscope->rte_capacity == 0 ? ROUTINES_INITIAL_CAPACITY : (gscope->rte_capacity*2);

        gscope->routines = realloc(gscope->routines, gscope->rte_capacity*sizeof(*gscope->routines));
    }

    gscope->routines[gscope->rte_count++] = routine;
}

// void gscope_append_variable()
// {
// }

// void gscope_search_variable()
// {
// }

void gscope_log(GScope *gscope)
{
    for (size_t j = 0; j < gscope->rte_count; ++j) {
        printf("ID: %s\n", gscope->routines[j]->id);
        for (size_t k = 0; k < gscope->routines[j]->tk_count; ++k) {
            printf("   ");
            tk_log(gscope->routines[j]->tokens[k]);
        }
    }
}

void gscope_destroy(GScope *gscope)
{
    for (size_t i = 0; i < gscope->rte_count; ++i) {
        rte_destroy(gscope->routines[i]);
    }

    free(gscope->routines);
    free(gscope);
}

void scan_modules(GScope *gscope, Module *mod) {
    // this function currently takes as input one single module
    // but in the future, when the module system will be implemented
    // it will accepts a dynamic array of modules

    const size_t mod_size = mod->count;

    for (size_t i = 0; i < mod_size; ++i) {
        Token *tk = mod->tokens[i];

        switch (tk->ttype) {

            case VAR_SYM: break;
            case ROUTINE_SYM: break;
            case ID_VAR: {

                assert(mod->tokens[i-1]->ttype == VAR_SYM);
                // TODO: do something with variable list
                assert(0 && "Variables not implemeted yet");

            } break;

            case ID_ROUTINE: {

                assert(mod->tokens[i-1]->ttype == ROUTINE_SYM);

                // create routine and fill tokens array
                Routine *routine = rte_create(tk->txt, TOKENS_INITIAL_CAPACITY);
                while (mod->tokens[(++i)-1]->ttype != KW_END) {
                    rte_append_token(routine, mod->tokens[i]);
                }

                gscope_append_routine(gscope, routine);

            } break;

            default: {
                assert(0 && "TokenType not allowed");
            }
        }

    }
}

#endif // INTERPRETER_H_
