#ifndef ARRAY_H_
#define ARRAY_H_

typedef struct {
    int size;
    int len;
    void** at;
} array_t;

int array_make(array_t* array_p);
int array_mextend(array_t* array_p);
int array_append(array_t* array_p, void* value);
int array_remove(array_t* array_p, int index);
void* array_get(array_t* array_p, int index);

#endif