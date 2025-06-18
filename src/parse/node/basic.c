#ifndef BASIC_C
#define BASIC_C

#include "generics.c"
#include "../node.c"

void AutoError(Node node) {
    fprintf(stderr, "Could not infer type \33[36mauto\33[0m\n");
}

void Auto(Node node, FILE* file) {
    fprintf(file, "int");
}

void Ignore(Node node, FILE* file) {}

char (*indent)[4] = 0;
int indent_depth = 0;

void BodyPrototype(Node node, FILE* file) {
    if(indent_depth++) push(&indent, "    ");
    for(int i = 0; i < len(node.body.children); i++) {
        Node* child = node.body.children[i];
        if(child->prototype != &Ignore) fprintf(file, "%.*s", (int) len(indent) * 4, (char*)(void*) indent);
        child->prototype(*child, file);
    }
    if(--indent_depth) pop(indent);
}

void CType(Node node, FILE* file) {
    fprintf(file, "%.*s", node.base_type.c_type.len, node.base_type.c_type.data);
}

Node void_type = {
    .prototype = &CType,
    .base_type = {
        .c_type = constr("void"),
    },
};

#endif