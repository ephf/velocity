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

Node* parse_binary_operation(str* tokenizer, Node* left, int precedence, int* continue_loop) {
    int compare_precedence;
    switch(tokenizer->id) {
        case '*': case '/': case '%': compare_precedence = 3; break;
        case '+': case '-': compare_precedence = 4; break;
        case '=': compare_precedence = 14; break;

        case 'O' /* :: */: {
            next(tokenizer);

            if(left->prototype == &Namespace) {
                str next_identifier = gimme(tokenizer, 'i');
                if(next_identifier.id < 0) return unexpected_token(*tokenizer);

                Node** value = get(left->namespace.body->body.context.variables, next_identifier);
                if(!*value) return cannot_find(next_identifier, "namespace variable");

                return *value;
            }

            return unexpected_token(*tokenizer);
        }

        default: {
            *continue_loop = 0;
            return left;
        }
    }

    if(precedence < compare_precedence) return left;
    str operator = next(tokenizer);
    Node* right = Expression(tokenizer, compare_precedence);

    if(!type_match(&left->type, right->type))
        return type_mismatch(left->type, right->type);

    return Box((Node) {
        .prototype = &BinaryOperation,
        .type = right->type,
        .binary_operation = {
            .left = left,
            .operator = operator,
            .right = right,
        },
    });
}

#endif