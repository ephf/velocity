#ifndef GENERICS_C
#define GENERICS_C

#include "../node.c"
#include "../include.h"

str stringify_generics(Vec(Node*) generics) {
    str identifier;
    for(int i = 0; i < len(generics); i++) {
        if(i == 0) identifier = stringify_type(generics[i]);
        else identifier = strc(identifier, constr("_"), stringify_type(generics[i]));
    }
    return identifier;
}

Node* ConformGenerics(Node* node) {
    Node* child = node->generic_wrapper.child;
    Vec(Node*) generics = node->generic_wrapper.generics;
    Vec(Node*)* generics_override = node->generic_wrapper.generics_override;
    str identifier = strc(node->generic_wrapper.identifier, constr("__"));
    Vec(str*) identifier_overrides = node->generic_wrapper.identifier_overrides;

    *generics_override = generics;
    identifier = strc(identifier, stringify_generics(generics));

    for(int i = 0; i < len(identifier_overrides); i++)
        *identifier_overrides[i] = identifier;

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

void GenericType(Node node, FILE* file) {
    Node* type = (*node.base_type.generic.generics)[node.base_type.generic.index];
    type->prototype(*type, file);
}

#endif