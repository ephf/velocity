#ifndef VARIABLE_C
#define VARIABLE_C

#include "../basic.c"
#include "../../include.h"

void Variable(Node node, FILE* file) {
    fprintf(file, "%.*s",
        node.variable.identifier.len, node.variable.identifier.data);
}

Node* find_variable(str token) {
    for(int i = len(stack); i--; ) {
        Node** variable = get(stack[i].variables, token);
        if(variable && !((*variable)->prototype == &Variable && (*variable)->variable.meta & vHidden))
            return *variable;
    }
    return cannot_find(token, "variable");
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

Node* parse_variable_declaration(str* tokenizer) {
    next(tokenizer);
    
    str identifier = gimme(tokenizer, 'i');
    if(identifier.id < 0) return unexpected_token(*tokenizer);

    Node* type = gimme(tokenizer, ':').id == ':'
        ? Type(tokenizer) : Box((Node) { .prototype = &Auto });

    switch(tokenizer->id) {
        case '=': {
            next(tokenizer);

            Node* value = Expression(tokenizer, 100);
            if(gimme(tokenizer, ';').id < 0) return unexpected_token(*tokenizer);

            type_match(&type, value->type);

            put(&stack[len(stack) - 1].variables, identifier, Box((Node) {
                .prototype = &Variable,
                .type = type,
                .variable = {
                    .identifier = identifier,
                },
            }));

            return Box((Node) {
                .prototype = &VariableDeclaration,
                .variable_declaration = {
                    .type = type,
                    .identifier = identifier,
                    .value = value,
                },
            });
        }

        case ';': {
            next(tokenizer);

            put(&stack[len(stack) - 1].variables, identifier, Box((Node) {
                .prototype = &Variable,
                .type = type,
                .variable = {
                    .identifier = identifier,
                },
            }));

            return Box((Node) {
                .prototype = &VariableDeclaration,
                .variable_declaration = {
                    .type = type,
                    .identifier = identifier,
                },
            });
        }

        default: return unexpected_token(*tokenizer);
    }
}

#endif