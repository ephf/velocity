#ifndef LIB
#define LIB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define len(v) ((v) ? ((int*)(void*)(v))[-1] / sizeof(*(v)) : 0)
#define pop(v) (((int*)(void*)(v))[-1] -= sizeof(*(v)))
#define push(v, i...) _push((int**)(void*)(v), &(typeof(**(v))[]) { i }, sizeof(**(v)))
int _push(int** v, void* i, int n) {
    if(!*v) (*v = (int*) calloc(sizeof(int[2]) + n, 1) + 2)[-2] = n;
    if(((*v)[-1] += n) > (*v)[-2]) {
        while(((*v)[-2] *= 2) < (*v)[-1]);
        *v = (int*) realloc(*v - 2, sizeof(int[2]) + (*v)[-2]) + 2;
    }
    memcpy((void*) *v + (*v)[-1] - n, i, n);
    return (*v)[-1] / n - 1;
}

#define Box(x...) ({ typeof(x) _x = (x); typeof(_x)* b = malloc(sizeof(_x)); memcpy(b, &_x, sizeof(_x)); b; })

char* readfile(char* n) {
    FILE* f = fopen(n, "r");
    fseek(f, 0, SEEK_END);
    char (*buf)[ftell(f)] = malloc(sizeof(*buf));
    rewind(f);
    (*buf)[fread(buf, 1, sizeof(*buf), f)] = 0;
    return (void*) buf;
}

#endif