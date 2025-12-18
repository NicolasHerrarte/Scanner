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
void initialize_##NAME(Array* array, CTYPE* src, size_t amount) {     \
    array->len = amount;                                              \
    array->elements = malloc(amount * sizeof(Element));               \
    for (size_t i = 0; i < amount; i++) {                             \
        Element e;                                                    \
        e.FIELD = src[i];                                             \
        array->elements[i] = e;                                       \
    }                                                                 \
}

#define DEFINE_ARRAY_PRINTER(NAME, FIELD)                              \
void print_##NAME(char* format, Array array) {                         \
    printf("[ ");                                                      \
    for(int i = 0;i<array.len-1;i++){                                  \
        printf(format, array.elements[i].FIELD);                       \
        printf(", ");                                                  \
    }                                                                  \
    printf(format, array.elements[array.len-1].FIELD);                 \
    printf(" ]\n");                                                    \
}

DEFINE_ARRAY_INITIALIZER(int,    int,    _int)
DEFINE_ARRAY_INITIALIZER(char,   char,   _char)
DEFINE_ARRAY_INITIALIZER(transition,    Transition,  _transition)

DEFINE_ARRAY_PRINTER(int,    _int)
DEFINE_ARRAY_PRINTER(char,   _char)

void initialize_empty(Array* array){
    array->elements = NULL;
    array->len = 0;
}

void append(Array* array, Element new_element){
    if(array->len == 0){
        array->elements = malloc(sizeof(Element));
        array->elements[0] = new_element;
        array->len = 1;
    }
    else{
        array->elements = realloc(array->elements, (array->len+1) * sizeof(Element));
        array->elements[array->len] = new_element;
        array->len += 1;
    }
}

void insert(Array *array, Element new_element, int position){
    assert(abs(position) <= array->len);
    if(position < 0){
        position = (array->len)+position;
    }
    if(array->len == 0){
        append(array, new_element);
    }
    else{
        array->elements = realloc(array->elements, (array->len+1) * sizeof(Element));
        for(int i = (array->len)-1;i>=position;i--){
            printf("%d\n", i);
            (array->elements)[i+1] = (array->elements)[i];
        }
        (array->elements)[position] = new_element;
        array->len += 1;
    }
}

void pop(Array *array, int position){
    assert(array->len > 0);
    if(position < 0){
        assert(position * -1 <= array->len);
        position = (array->len)+position;
    }
    else{
        assert(position < array->len);
    }
    for(int i = position+1;i<array->len;i++){
        printf("%d\n", i);
        (array->elements)[i-1] = (array->elements)[i];
    }
    array->elements = realloc(array->elements, (array->len-1) * sizeof(Element));
    array->len -= 1;
}

Element get(Array *array, int index){
    if(index < 0){
        assert(index * -1 <= array->len);
        index = (array->len)+index;
    }
    else{
        assert(index < array->len);
    }
    return (array->elements)[index];
}
