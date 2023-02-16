#include <stdlib.h>
#include <string.h>

#include "array.h"

int array_make(array_t* array_p) {
    array_p->len = 0;
    array_p->size = 1;

    void** content_p = (void**) malloc(array_p->size * sizeof(void*));
    if (!content_p) return 1;
    array_p->at = content_p;

    return 0;
}

int array_mextend(array_t* array_p) {
    array_p->size *= 2;
    void** content_p = (void**) realloc(array_p->at, array_p->size * sizeof(void*));
    if (!content_p) return 1;
    array_p->at = content_p;

    return 0;
}

int array_append(array_t* array_p, void* value) {
    if (array_p->len == array_p->size) array_mextend(array_p);

    array_p->at[array_p->len] = value;
    array_p->len++;

    return 0;
}

int array_remove(array_t* array_p, int index) {
    if (index >= array_p->len) return 1;
    array_p->at[array_p->len] = 0;
    for (int i = index + 1; i < array_p->len; i++) {
        array_p->at[i - 1] = array_p->at[i];
        array_p->at[i] = 0;
    }

    array_p->len--;

    return 0;
}

void* array_get(array_t* array_p, int index) {
    return array_p->at[index];
}