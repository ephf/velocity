#ifndef BINARY_OPERATION_C
#define BINARY_OPERATION_C

#include "../basic.c"
#include "../../include.h"

void BinaryOperation(Node node, FILE* file) {
    node.binary_operation.left->prototype(*node.binary_operation.left, file);
    fprintf(file, " %.*s ", node.binary_operation.operator.len,
        node.binary_operation.operator.data);
    node.binary_operation.right->prototype(*node.binary_operation.right, file);
}

Node parse_binary_operation(str* tokenizer, Node left, int precedence, int* continue_loop) {
    int compare_precedence;
    switch(tokenizer->id) {
        case '*': case '/': case '%': compare_precedence = 3; break;
        case '+': case '-': compare_precedence = 4; break;
        case 'H': case 'J': compare_precedence = 5; break;
        case '=': compare_precedence = 14; break;
        default: {
            *continue_loop = 0;
            return left;
        }
    }

    if(precedence < compare_precedence) return left;
    str operator = next(tokenizer);
    Node right = Expression(tokenizer, compare_precedence);

    if(!type_match(&left.type, right.type))
        return type_mismatch(Box(left), Box(right));

    return (Node) {
        .prototype = &BinaryOperation,
        .type = right.type,
        .binary_operation = {
            .left = Box(left),
            .operator = operator,
            .right = Box(right),
        },
    };
}

#endif