#ifndef STACK_H_
#define STACK_H_
#include <stdlib.h>

#define DEFAULT_INITIAL_CAPACITY 5

typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} Stack;

Stack st_create_on_stack(const size_t initial_capacity)
{
    Stack stack = {0};
    stack.capacity = initial_capacity;
    stack.items = calloc(initial_capacity, sizeof(*stack.items));
    return stack;
}

Stack *st_create_on_heap(const size_t initial_capacity)
{
    Stack *stack = malloc(sizeof(Stack));
    stack->capacity = initial_capacity;
    stack->items = calloc(initial_capacity, sizeof(*stack->items));
    return stack;
}

void st_push(Stack *stack, char *item)
{
    // if count is equal to capacity reallocate memory using more space
    if (stack->count == stack->capacity) {
        stack->capacity = stack->capacity == 0 ? DEFAULT_INITIAL_CAPACITY : (stack->capacity*2);
        stack->items = realloc(stack->items, stack->capacity*sizeof(*stack->items));
        assert(stack->items != NULL && "Are you serious? Out of RAM");
    }

    // this function manages memory on its own
    *(stack->items+stack->count) = malloc(strlen(item));

    // it doesn't matter where item argument is allocated, it will be copied
    strcpy(*(stack->items+stack->count++), item);
}

char *st_peek(Stack *stack)
{
    return *(stack->items+stack->count-1);
}

void st_pop(Stack *stack)
{
    // this function is intended to be used right after st_peek(st) if you
    // want to get top element before deleting
    free(*(stack->items+stack->count--));
}

void st_destroy_from_stack(Stack *stack)
{
    // deallocate memory on the heap
    for (size_t i = 0; i < stack->count; ++i)
        free(*(stack->items+i));
    free(stack->items);
}

void st_destroy_from_heap(Stack *stack)
{
    st_destroy_from_stack(stack);

    // also deallocate struct
    free(stack);
}


void st_display(Stack *stack)
{
    if (stack->count != 0) {
        char **begin_pt = stack->items;
        char **end_pt = begin_pt + stack->count-1; 

        printf("[");
        while (end_pt != begin_pt)
            printf("%s, ", *end_pt--);
        printf("%s]\n", *begin_pt);
    } else printf("[]\n");
}

#endif  // STACK_H_
