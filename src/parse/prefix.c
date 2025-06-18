#include "literal.c"

void Dereference(Node node, FILE* file) {
    fprintf(file, "(*");
    node.dereference->prototype(*node.dereference, file);
    fprintf(file, ")");
}

Node* dereference(Node* node) {
    return Box((Node) { &Dereference, 0, node->range,
        .type = dereference_type(node->range, node->type),
        .dereference = node,
    });
}

void Reference(Node node, FILE* file) {
    fprintf(file, "(&");
    node.reference->prototype(*node.reference, file);
    fprintf(file, ")"); 
}

Node* reference(Node* node) {
    return Box((Node) { &Reference, 0, node->range,
        .type = reference_type(node->range, node->type),
        .reference = node,
    });
}

Node* Prefix(Tokenizer* tokenizer) {
    switch(tokenizer->current.type) {
        case '*': {
            next_token(tokenizer);
            return dereference(Literal(tokenizer));
        }

        case '&': {
            next_token(tokenizer);
            return reference(Literal(tokenizer));
        }
    }

    return Literal(tokenizer);
}