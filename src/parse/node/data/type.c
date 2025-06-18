#ifndef TYPE_C
#define TYPE_C

#include "../basic.c"
#include "../symbol/identifier.c"

Node* parse_type_declaration(Tokenizer* tokenizer) {
    TokenResult range_start = next_token(tokenizer);
    ResolvedIdentifier ri = resolve_identifier(tokenizer, 1);
    
    expect_token(tokenizer, '=');
    Node* type = Type(tokenizer);

    expect_token(tokenizer, ';');
    put(&stack[len(stack) - 1]->types, ri.base.str, type);
    return Box((Node) { &Ignore });
}

#endif