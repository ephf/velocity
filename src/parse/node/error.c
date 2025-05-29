#ifndef ERROR_C
#define ERROR_C

#include "../node.c"

int errc = 0;
void Error(Node node, FILE* file) {
    fprintf(file, "<error (%d)>", ++errc);
    fprintf(stderr, "(%d) \33[31mE:\33[0m ", errc);
    node.error.perror(node);
}

void ErrorType(Node node) {
    fprintf(stderr, "Error parsing type\n");
}

Node error_type = {
    .prototype = &Error,
    .type = &error_type,
    .error = {
        .perror = &ErrorType,
    }
};

void UnexpectedTokenError(Node node) {
    fprintf(stderr, "Unexpected token \33[36m%.*s\33[0m\n",
        node.error.label.len, node.error.label.data);
}

Node unexpected_token(str token) {
    return (Node) {
        .prototype = &Error,
        .type = &error_type,
        .error = {
            .perror = &UnexpectedTokenError,
            .label = token,
        },
    };
}

void CannotFindError(Node node) {
    fprintf(stderr, "Cannot find %s \33[36m%.*s\33[0m in scope\n",
        node.error.cannot_find, node.error.label.len, node.error.label.data);
}

Node cannot_find(str token, char* cannot_find) {
    return (Node) {
        .prototype = &Error,
        .type = &error_type,
        .error = {
            .perror = &CannotFindError,
            .cannot_find = cannot_find,
            .label = token,
        },
    };
}

void TypeMismatchError(Node node) {
    fprintf(stderr, "Type mismatch between \33[36m");
    node.error.mismatch.a->prototype(*node.error.mismatch.a, stderr);
    fprintf(stderr, "\33[0m and \33[36m");
    node.error.mismatch.b->prototype(*node.error.mismatch.b, stderr);
    fprintf(stderr, "\33[0m\n");
}

Node type_mismatch(Node* a, Node* b) {
    return (Node) {
        .prototype = &Error,
        .type = &error_type,
        .error = {
            .perror = &TypeMismatchError,
            .mismatch = {
                .a = a,
                .b = b,
            },
        },
    };
}

#endif