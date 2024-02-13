#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define DEBUG
#define ERR_PREFIX "ERROR %s:%d: "          // error prefix for file path and line number
#define ERR_EXP __FILE__, __LINE__    // arguments expansion

#include "stack.h"
#include "lexer.h"
#include "interpreter.h"

size_t get_file_content_length(FILE *file_pointer)
{
    size_t content_length = 0;

    // move file pointer to end of file
    fseek(file_pointer, 0, SEEK_END);

    // retrieve number of bytes
    content_length = (size_t) ftell(file_pointer);

    // reset file pointer position to start
    fseek(file_pointer, 0, SEEK_SET);
    return content_length;
}

FILE *open_file(const char *file_path)
{
    // open file in reading mode
    FILE *file_pointer = fopen(file_path, "r");
    if (file_pointer == NULL) {
        fprintf(stderr, ERR_PREFIX"Could not open file: %s\n", ERR_EXP, file_path);
        exit(EXIT_FAILURE);
    }
    return file_pointer;
}

char *read_content_from_file(const char *file_path)
{
    FILE *file_pointer = open_file(file_path);

    // consider null terminating character
    size_t buffer_size = get_file_content_length(file_pointer)+1;

    // allocate necessary memory
    char* buffer = calloc(1, buffer_size);
    if (buffer == NULL) {
        fprintf(stderr, ERR_PREFIX"Could not allocate memory\n", ERR_EXP);
        exit(EXIT_FAILURE);
    }

    // read 1*buffer_size bytes from file
    size_t read_bytes = fread(buffer, 1, buffer_size, file_pointer);
    assert(read_bytes == buffer_size-1 && "fread() failed: read bytes "
           "differs from expected number of bytes");

    fclose(file_pointer);
    return buffer;
}

void build_enums_str_reprs()
{
    BUILD_ENUM_STR_REPR(token_types_enum_str_repr, _IOTA, TokenType, ttype_tostr);
    BUILD_ENUM_STR_REPR(value_types_enum_str_repr, VT_IOTA, ValueType, vtype_tostr);
}

#define MEM_CAPACITY 128
#define FILE_PATH "examples/conditionals.pc"

int main()
{
    char *buffer = read_content_from_file(FILE_PATH);
    Module *mod = lex_buffer(buffer, FILE_PATH);

#ifdef DEBUG

    printf(">>>>>>> [LEX WORK]\n");
    mod_log(mod);
    printf("=========================================================\n");

#endif // DEBUG

    GScope *gscope = gscope_create(GSCOPE_ROUTINES_INITIAL_CAPACITY, GSCOPE_VARIABLES_INITIAL_CAPACITY);
    
    scan_modules(gscope, mod);
#ifdef DEBUG
    printf(">>>>>>> [VARIABLES]\n");
    gscope_log_variables(gscope);
    printf("=========================================================\n");
    printf(">>>>>>> [ROUTINES]\n");
    gscope_log_routines(gscope);
    printf("=========================================================\n");
#endif // DEBUG

    Stack *mem = st_create_on_heap(MEM_CAPACITY);
    size_t main_rte = gscope_search_routine(gscope, "main");
    rte_execute(gscope->routines[main_rte], mem, gscope);

    return EXIT_SUCCESS;
}
