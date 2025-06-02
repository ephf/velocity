#ifndef TYPE_C
#define TYPE_C

#include "../basic.c"

Node* parse_type_declaration(str* tokenizer) {
    next(tokenizer);

    str identifier = gimme(tokenizer, 'i');
    if(identifier.id < 0) return unexpected_token(*tokenizer);
    
    if(gimme(tokenizer, '=').id < 0) return unexpected_token(*tokenizer);
    Node* type = Type(tokenizer);
    if(type->prototype == &Error) return type;

    if(gimme(tokenizer, ';').id < 0) return unexpected_token(*tokenizer);
    put(&stack[len(stack) - 1].types, identifier, type);
    return Box((Node) { .prototype = &Ignore });
}

#endif