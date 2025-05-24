#include "prefix.c"

void BinaryOperation(Node node, FILE* file) {
    node.binary_operation.left->prototype(*node.binary_operation.left, file);
    fprintf(file, " %.*s ", node.binary_operation.operator.len,
        node.binary_operation.operator.data);
    node.binary_operation.right->prototype(*node.binary_operation.right, file);
}

void FunctionCall(Node node, FILE* file) {
    Node function = *node.function_call.function;
    Node* arguments = node.function_call.arguments;

    fprintf(file, "%.*s(", function.variable.identifier.len, 
        function.variable.identifier.data);
    for(int i = 0; i < len(arguments); i++) {
        arguments[i].prototype(arguments[i], file);
        if(i != len(arguments) - 1) fprintf(file, ", ");
    }
    fprintf(file, ")");
}

Node Postfix(str* tokenizer, Node left, int order) {
    while(1) {
        int compare_order;
        switch(tokenizer->id) {
            case '*': case '/': case '%': compare_order = 3; break;
            case '+': case '-': compare_order = 4; break;
            case 'H': case 'J': compare_order = 5; break;
            case '=': compare_order = 14; break;

            case '(': {
                next(tokenizer);
                Node* arguments = 0;

                while(tokenizer->id != ')') {
                    push(&arguments, Postfix(tokenizer, Prefix(tokenizer), 100));
                    if(tokenizer->id == ',') next(tokenizer);
                    else break;
                }
                if(gimme(tokenizer, ')').id < 0) return (Node) {
                    .prototype = &Error,
                    .error = {
                        .perror = &UnexpectedToken,
                        .label = next(tokenizer),
                    },
                };

                return (Node) {
                    .prototype = &FunctionCall,
                    .type = left.type->defined_type.declaration->function_declaration.signature[0].type,
                    .function_call = {
                        .function = Box(left),
                        .arguments = arguments,
                    },
                };
            }

            default: return left;
        }

        if(order < compare_order) return left;
        str operator = next(tokenizer);
        Node right = Postfix(tokenizer, Prefix(tokenizer), compare_order);

        Node binary_operation = {
            .prototype = &BinaryOperation,
            .type = right.type,
            .binary_operation = {
                .left = Box(left),
                .operator = operator,
                .right = Box(right),
            },
        };

        if(!type_match(left.type, *right.type)) {
            return (Node) {
                .prototype = &Error,
                .type = error_type,
                .error = {
                    .perror = &TypeMismatch,
                    .mismatch = {
                        .a = binary_operation.binary_operation.left->type,
                        .b = binary_operation.binary_operation.right->type,
                    }
                },
            };
        }

        left = binary_operation;
    }

    return left;
}