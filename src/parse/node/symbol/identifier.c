#ifndef IDENTIFIER_C
#define IDENTIFIER_C

#include "../basic.c"
#include "../statement/namespace.c"

typedef struct {
    Token base;
    str constructed;
    Context* scope;
    bool caused_error;
} ResolvedIdentifier;

Node* find_variable(str);

ResolvedIdentifier resolve_identifier_given(Token identifier, Tokenizer* tokenizer, int load_scope) {
    ResolvedIdentifier ri = { identifier, identifier.str };

    while(try_token(tokenizer, TokenDoubleColon, 0)) {
        TokenResult next_identifier = expect_token(tokenizer, TokenIdentifier);
        if(next_identifier.is_error) {
            ri.caused_error = true;
            continue;
        }

        Node* namespace = ri.scope ? *(get(ri.scope->variables, ri.base.str) ?: &(Node*) { 0 }) : find_variable(ri.base.str);
        if(!namespace || namespace->prototype != &Namespace) {
            ri.caused_error = true;
            push(&errors, cannot_find(ri.base, "namespace"));
        }
        
        ri.scope = &namespace->namespace.body->body.context;
        ri.constructed = strc(ri.constructed, constr("__"), next_identifier.token.str);
        ri.base = next_identifier.token;
    }

    if(stack[len(stack) - 1]->namespace)
        ri.constructed = strc(stack[len(stack) - 1]->namespace->namespace.identifier, constr("__"), ri.constructed);

    if(load_scope && !ri.scope) ri.scope = stack[len(stack) - 1];
    return ri;
}

ResolvedIdentifier resolve_identifier(Tokenizer* tokenizer, int load_scope) {
    TokenResult identifier = expect_token(tokenizer, TokenIdentifier);
    ResolvedIdentifier ri = resolve_identifier_given(identifier.token, tokenizer, load_scope);
    if(identifier.is_error) ri.caused_error = true;
    return ri;
}

#endif