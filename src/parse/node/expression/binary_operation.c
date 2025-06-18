#ifndef BINARY_OPERATION_C
#define BINARY_OPERATION_C

#include "../basic.c"
#include "../data/variable.c"
#include "../statement/namespace.c"
#include "../../include.h"

void BinaryOperation(Node node, FILE* file) {
    node.binary_operation.left->prototype(*node.binary_operation.left, file);
    fprintf(file, " %.*s ", node.binary_operation.operator.len,
        node.binary_operation.operator.data);
    node.binary_operation.right->prototype(*node.binary_operation.right, file);
}

Node* parse_binary_operation(Tokenizer* tokenizer, Node* left, int precedence, int* continue_loop) {
    int compare_precedence;
    switch(tokenizer->current.type) {
        case '*': case '/': case '%': compare_precedence = 3; break;
        case '+': case '-': compare_precedence = 4; break;
        case '=': compare_precedence = 14; break;

        default: {
            *continue_loop = 0;
            return left;
        }
    }

    if(precedence < compare_precedence) return left;

    TokenResult operator = next_token(tokenizer);
    Node* right = Expression(tokenizer, compare_precedence);
    Token operation_range = stretch_token(left->range, right->range);

    if(!type_match(&left->type, right->type))
        push(&errors, type_mismatch(operation_range, left->type, right->type));

    return Box((Node) { &BinaryOperation, 0, operation_range,
        .type = right->type,
        .binary_operation = { left, operator.token.str, right }
    });
}

#endif