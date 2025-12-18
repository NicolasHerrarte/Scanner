#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

typedef struct Transition{
    int state_from;
    int state_to;
    char trans_char;
} Transition;

typedef union Element {
    int _int;
    float _float;
    double _double;
    char _char;
    Transition _transition;
    void* _ptr;
} Element;

typedef struct Array{
    Element* elements;
    int len;
} Array;

#define DEFINE_ARRAY_INITIALIZER(NAME, CTYPE, FIELD)                  \
void initialize_##NAME(Array* array, CTYPE* src, size_t amount);

#define DEFINE_ARRAY_PRINTER(NAME, FIELD)                              \
void print_##NAME(char* format, Array array);

DEFINE_ARRAY_INITIALIZER(int,    int,    _int)
DEFINE_ARRAY_INITIALIZER(char,   char,   _char)
DEFINE_ARRAY_INITIALIZER(transition,    Transition,  _transition)

DEFINE_ARRAY_PRINTER(int,    _int)
DEFINE_ARRAY_PRINTER(char,   _char)

void initialize_empty(Array* array);

void append(Array* array, Element new_element);

void insert(Array *array, Element new_element, int position);

void pop(Array *array, int position);

Element get(Array *array, int index);