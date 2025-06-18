#ifndef VARIABLE_C
#define VARIABLE_C

#include "../basic.c"
#include "../../include.h"
#include "../symbol/identifier.c"

void Variable(Node node, FILE* file) {
    fprintf(file, "%.*s",
        node.variable.identifier.len, node.variable.identifier.data);
}

Node* find_variable(str identifier) {
    for(int i = len(stack); i--; ) {
        Node** variable = get(stack[i]->variables, identifier);
        if(variable && !((*variable)->prototype == &Variable && (*variable)->attributes & aHidden))
            return *variable;
    }
    return 0;
}

void VariableDeclaration(Node node, FILE* file) {
    Node type = *node.variable_declaration.type;
    str identifier = node.variable_declaration.identifier;
    Node* value = node.variable_declaration.value;

    type.prototype(type, file);
    fprintf(file, " %.*s", identifier.len, identifier.data);
    if(value) {
        fprintf(file, " = ");
        value->prototype(*value, file);
    }
    fprintf(file, ";\n");
}

void Namespace(Node, FILE*);

Node* parse_variable_declaration(Tokenizer* tokenizer) {
    int attributes = 0;
    TokenResult declarator = next_token(tokenizer);
    if(streq(declarator.token.str, constr("const"))) attributes |= aConst;
    
    ResolvedIdentifier ri = resolve_identifier(tokenizer, 1);
    Node* type = try_token(tokenizer, ':', 0)
        ? Type(tokenizer) : Box((Node) { &Auto });

    Node* value = 0;
    if(try_token(tokenizer, '=', 0)) {
        value = Expression(tokenizer, 100);

        if(!type_match(&type, value->type)) push(&errors, type_mismatch(
            stretch_token(declarator.token, value->range), type, value->type));
    }
    expect_token(tokenizer, ';');

    put(&stack[len(stack) - 1]->variables, ri.base.str, attributes & aConst
        ? value : Box((Node) { &Variable, attributes, ri.base,
            .type = type,
            .variable = { ri.constructed },
        })); 

    return attributes & aConst
        ? Box((Node) { &Ignore })
        : Box((Node) { &VariableDeclaration, attributes, stretch_token(declarator.token, ri.base),
            .variable_declaration = { type, ri.constructed, value }
        });
}

#endif