#ifndef BASIC_C
#define BASIC_C

#include "error.c"
#include "generics.c"
#include "../node.c"

void AutoError(Node node) {
    fprintf(stderr, "Could not infer type \33[36mauto\33[0m\n");
}

void Auto(Node node, FILE* file) {
    if(node.type_meta & tIsNumeric) {
        fprintf(file, "int");
        return;
    }

    Error((Node) {
        .error = {
            .perror = &AutoError,
        }
    }, file);
}

char (*indent)[4] = 0;
int initial_indent = 0;

void BodyPrototype(Node node, FILE* file) {
    if(initial_indent++) push(&indent, "    ");
    for(int i = 0; i < len(node.body.children); i++) {
        fprintf(file, "%.*s", (int) len(indent) * 4, (char*)(void*) indent);
        Node* child = node.body.children[i];
        child->prototype(*child, file);
    }
    if(--initial_indent) pop(indent);
}

void Ignore(Node node, FILE* file) {}

void CType(Node node, FILE* file) {
    fprintf(file, "%.*s", node.base_type.c_type.len, node.base_type.c_type.data);
}

#endif