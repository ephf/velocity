#ifndef BASIC_C
#define BASIC_C

#include "error.c"
#include "../node.c"

void AutoError(Node node) {
    fprintf(stderr, "Could not infer type \33[36mauto\33[0m\n");
}

void Auto(Node node, FILE* file) {
    Error((Node) {
        .error = {
            .perror = &AutoError,
        }
    }, file);
}

void Ignore(Node node, FILE* file) {}

void CType(Node node, FILE* file) {
    fprintf(file, "%s", node.base_type.c_type);
}

#define ctype(type) \
    ((Node) { \
        .prototype = &CType, \
        .type_meta = tIsNumeric, \
        .base_type = { \
            .c_type = type, \
        }, \
    })

__attribute__ ((constructor))
void init_std_types() {
    Context internal_context = { 0 };

    internal_context.types = map(str, Node, {
        { constr("int"), ctype("int") },
        { constr("usize"), ctype("size_t") },
        { constr("char"), ctype("char") },
        { constr("void"), ctype("void") },
    });

    push(&stack, internal_context);
}

#endif