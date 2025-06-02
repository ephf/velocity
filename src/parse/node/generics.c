#ifndef GENERICS_C
#define GENERICS_C

#include "../node.c"
#include "../include.h"

Node* ConformGenerics(Node* node) {
    Node* child = node->generic_wrapper.child;
    Vec(Node*) generics = node->generic_wrapper.generics;
    Vec(Node*)* generics_override = node->generic_wrapper.generics_override;
    str identifier = strc(node->generic_wrapper.identifier, constr("_"));
    Vec(str*) identifier_overrides = node->generic_wrapper.identifier_overrides;

    *generics_override = generics;
    for(int i = 0; i < len(generics); i++) {
        identifier = strc(identifier, constr("_"), stringify_type(generics[i]));
    }

    for(int i = 0; i < len(identifier_overrides); i++) {
        *identifier_overrides[i] = identifier;
    }

    return child;
}

void GenericWrapper(Node node, FILE* file) {
    Node* child = ConformGenerics(&node);
    child->prototype(*child, file);
}

void GenericTypeWrapper(Node node, FILE* file) {
    ConformGenerics(node.base_type.generic_wrapper.parent);
    Node* child = node.base_type.generic_wrapper.child;
    child->prototype(*child, file);
}

#endif