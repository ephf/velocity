#ifndef NAMESPACE_C
#define NAMESPACE_C

#include "../basic.c"

void Namespace(Node node, FILE* file) {
    fprintf(file, "%.*s", node.namespace.identifier.len,
        node.namespace.identifier.data);
}

#endif