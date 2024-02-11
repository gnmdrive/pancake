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

void retrieve_last_numbers_int(int *x, int *y, Stack *mem)
{
    // peek last and second-last items from stack
    char *a = st_peek(mem, 0);
    char *b = st_peek(mem, 1);

    int x_res = (int) strtol(a, (char **) NULL, 10);
    int y_res = (int) strtol(b, (char **) NULL, 10);


    st_pop(mem);
    st_pop(mem);

    *x = x_res;
    *y = y_res;
}

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

void interpret_lex_outcome(LexOutcome *lex_outcome, Stack *mem)
{
    const size_t lo_size = lex_outcome->count;
    bool entry_point = false;

    for (size_t i = 0; i < lo_size; ++i) {
        Token *tk = lex_outcome->tokens[i];
        // tk_log(tk);

        if (entry_point) {
            // execute code inside main routine

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

                case ROUTINE_END: {

                    // main routine stack should be empty at program end
                    assert(mem->count == 0);

                    // TODO: should to something
                } break;

                default: {
                    fprintf(stderr, ERR_PREFIX"Can't interpret this token: '%s'\n", ERR_EXP, tk->txt);
                    assert(0);
                    // exit(EXIT_FAILURE);
                } break;
            }

        }

        if (tk->ttype == ID_VAR) {
            // TODO: do something with variable list
            assert(lex_outcome->tokens[i-1]->ttype == VAR_SYM);
            if (strcmp(tk->txt, "main") == 0) {
                fprintf(stderr, "ERROR: can't create variable with name 'main'\n");
                exit(EXIT_FAILURE);
            }
        }

        if (tk->ttype == ID_ROUTINE) {
            assert(lex_outcome->tokens[i-1]->ttype == ROUTINE_SYM);

            // found entry point, start executing code from next iteration
            entry_point = strcmp(tk->txt, "main") == 0;
        }

    }

    if (!entry_point) {
        fprintf(stderr, "ERROR: you must provide an entry point\n");
        exit(EXIT_FAILURE);
    }
}

#endif // INTERPRETER_H_
