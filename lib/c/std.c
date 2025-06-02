#ifndef VELOCITY_C_STD
#define VELOCITY_C_STD

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct str {
    char* data;
    size_t size;
};

void print(struct str x) {
    printf("%.*s\n", (int) x.size, x.data);
}

#endif