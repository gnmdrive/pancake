#ifndef STACK_H_
#define STACK_H_
#include <stdlib.h>

#define DEFAULT_STACK_INITIAL_CAPACITY 16

typedef enum {
    VT_UNKNOWN,
    VT_STRING,
    VT_INT,
    VT_FLOAT,
    VT_BOOL,
    VT_IOTA,
} ValueType;

char *vtype_tostr(ValueType vtype)
{
    switch (vtype) {
        case VT_STRING:
            return "VT_STRING";
            break;
        case VT_INT:
            return "VT_INT";
            break;
        case VT_FLOAT:
            return "VT_FLOAT";
            break;
        case VT_BOOL:
            return "VT_BOOL";
            break;
        default:
            assert(0 && "Missing one or multiple ValueType in enum");
            break;
    }
}

char **value_types_enum_str_repr;

typedef struct {
    char *txt;
    ValueType type;
} Value;

Value *value_create(char *txt, ValueType vtype)
{
    Value *value = malloc(sizeof(ValueType));

    // this funciton manage memory on its own
    value->txt = malloc(strlen(txt));
    strcpy(value->txt, txt);

    value->type = vtype;
    return value;
}

void value_log(Value *value)
{
    printf("(%s) %s\n", vtype_tostr(value->type), value->txt);
}

void value_destroy(Value *value)
{
    free(value->txt);
    free(value);
}

typedef struct {
    Value **items;
    size_t count;
    size_t capacity;
} Stack;

Stack st_create_on_stack(const size_t initial_capacity)
{
    Stack stack = {0};
    stack.capacity = initial_capacity;

    stack.items = calloc(initial_capacity, sizeof(*stack.items));
    if (stack.items == NULL) {
        fprintf(stderr, ERR_PREFIX"Could not allocate memory\n", ERR_EXP);
        exit(EXIT_FAILURE);
    }

    return stack;
}

Stack *st_create_on_heap(const size_t initial_capacity)
{
    Stack *stack = malloc(sizeof(Stack));
    if (stack == NULL) {
        fprintf(stderr, ERR_PREFIX"Could not allocate memory\n", ERR_EXP);
        exit(EXIT_FAILURE);
    }

    stack->count = 0;
    stack->capacity = initial_capacity;
    stack->items = calloc(initial_capacity, sizeof(*stack->items));
    if (stack->items == NULL) {
        fprintf(stderr, ERR_PREFIX"Could not allocate memory\n", ERR_EXP);
        exit(EXIT_FAILURE);
    }

    return stack;
}

void st_push(Stack *stack, Value *item)
{
    // if count is equal to capacity reallocate memory using more space
    if (stack->count == stack->capacity) {
        stack->capacity = stack->capacity == 0 ? DEFAULT_STACK_INITIAL_CAPACITY : (stack->capacity*2);
        stack->items = realloc(stack->items, stack->capacity*sizeof(*stack->items));
        if (stack->items == NULL) {
            fprintf(stderr, ERR_PREFIX"Could not allocate memory\n", ERR_EXP);
            exit(EXIT_FAILURE);
        }
    }

    stack->items[stack->count++] = item;
}

Value *st_peek(Stack *stack, size_t n)
{
    return stack->items[stack->count-1-n];
}

void st_pop(Stack *stack)
{
    // this function is intended to be used right after st_peek(st, n) if you
    // want to get top element before deleting
    value_destroy(stack->items[stack->count-1]);
    stack->count--;
}

void st_destroy_from_stack(Stack *stack)
{
    // deallocate memory on the heap
    for (size_t i = 0; i < stack->count; ++i)
        value_destroy(stack->items[i]);
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
        size_t i = 0;
        size_t stack_count = stack->count-1;
        printf("[");
        while (i < stack_count) {
            printf("(%s) %s, ", vtype_tostr(stack->items[i]->type), stack->items[i]->txt);
            i++;
        }
        printf("(%s) %s <-\n", vtype_tostr(stack->items[i]->type), stack->items[i]->txt);
    } else printf("[ <-\n");
}

#endif  // STACK_H_
