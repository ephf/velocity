#include "prefix.c"

#include "node/expression/binary_operation.c"
#include "node/data/function.c"

Node* Expression(str*, int);

Node* Postfix(str* tokenizer, Node* left, int precedence) {
    int continue_loop = 1;
    while(continue_loop) {
        if(left->prototype == &Variable && left->type->prototype == &FunctionType)
            organize_function_generics(tokenizer, left);

        switch(tokenizer->id) {
            case '(': left = parse_function_call(tokenizer, left); break;
        }

        left = parse_binary_operation(tokenizer, left, precedence, &continue_loop);
        if(left->prototype == &Error) return left;
    }

    return left;
}

Node* Expression(str* tokenizer, int precedence) {
    return Postfix(tokenizer, Prefix(tokenizer), precedence);
}