#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "stack.h"

void raise_generic_error(const char *message, const char *reason)
{
    fprintf(stderr, message, reason);
    exit(EXIT_FAILURE);
}

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
    if (file_pointer == NULL) raise_generic_error("Could not open file: %s\n", file_path);
    return file_pointer;
}

char *read_content_from_file(const char *file_path)
{
    FILE *file_pointer = open_file(file_path);

    // consider null terminating character
    size_t buffer_size = get_file_content_length(file_pointer)+1;

    // allocate necessary memory
    char* buffer = calloc(1, buffer_size);
    if (buffer == NULL) raise_generic_error("Could not allocate memory\n", NULL);

    // read 1*buffer_size bytes from file
    size_t read_bytes = fread(buffer, 1, buffer_size, file_pointer);
    assert(read_bytes == buffer_size-1 && "fread() failed: read bytes "
           "differs from expected number of bytes");

    fclose(file_pointer);
    return buffer;
}

#define MEM_CAPACITY 128
#define FILE_PATH "examples/code.pc"

int main()
{
    Stack *mem = st_create_on_heap(MEM_CAPACITY);
    char *buffer = read_content_from_file(FILE_PATH);
    printf("hello from pancake\n");

    return EXIT_SUCCESS;
}
