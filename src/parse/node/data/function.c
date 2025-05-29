#ifndef FUNCTION_C
#define FUNCTION_C

#include "../basic.c"
#include "variable.c"
#include "../../include.h"

void FunctionType(Node node, FILE* file) {
    Vec(Node*) signature = node.base_type.declaration->function_declaration.signature;
    fprintf(file, "typeof(");
    signature[0]->prototype(*signature[0], file);
    fprintf(file, "(*)())");
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

void FunctionDeclaration(Node node, FILE* file) {
    Vec(Node*) signature = node.function_declaration.signature;
    str identifier = node.function_declaration.identifier;
    Node* body = node.function_declaration.body;
    
    signature[0]->prototype(*signature[0], file);
    fprintf(file, " %.*s() {\n", identifier.len, identifier.data);
    body->prototype(*body, file);
    fprintf(file, "%.*s}\n", (int) len(indent) * 4, (char*)(void*) indent);
}

Node parse_function_call(str* tokenizer, Node left) {
    if(left.prototype != &Variable || left.type->prototype != &FunctionType)
        return type_mismatch(Box(left), Box((Node) {
            .prototype = &FunctionType,
            .base_type = {
                .declaration = Box((Node) {
                    .prototype = &FunctionDeclaration,
                    .function_declaration = {
                        .signature = vec(Box(ctype("void"))),
                    },
                }),
            },
        }));

    next(tokenizer);
    Node* arguments = 0;

    while(tokenizer->id != ')') {
        push(&arguments, Expression(tokenizer, 100));
        if(tokenizer->id == ',') next(tokenizer);
        else break;
    }
    if(gimme(tokenizer, ')').id < 0) return unexpected_token(*tokenizer);

    return (Node) {
        .prototype = &FunctionCall,
        .type = left.type->base_type.declaration->function_declaration.signature[0]->type,
        .function_call = {
            .function = Box(left),
            .arguments = arguments,
        },
    };
}

Node parse_function_declaration(str* tokenizer) {
    next(tokenizer);

    str identifier = gimme(tokenizer, 'i');
    Vec(Node*) signature = vec((Node*) 0);
    str* argument_names = 0;

    if(identifier.id < 0 || gimme(tokenizer, '(').id < 0)
        return unexpected_token(*tokenizer);

    while(tokenizer->id != ')') {
        str argument_name = gimme(tokenizer, 'i');
        if(argument_name.id < 0 || gimme(tokenizer, ':').id < 0)
            return unexpected_token(*tokenizer);
        Node* argument_type = Type(tokenizer);

        push(&signature, argument_type);
        push(&argument_names, argument_name);

        if(tokenizer->id == ',') next(tokenizer);
        else break;
    }
    if(gimme(tokenizer, ')').id < 0) return unexpected_token(*tokenizer);

    if(tokenizer->id == 'r') {
        next(tokenizer);
        signature[0] = Type(tokenizer);
    } else {
        signature[0] = Box(ctype("void"));
    }

    if(tokenizer->id == 'i' && streq(*tokenizer, constr("__external"))) {
        next(tokenizer);
        if(gimme(tokenizer, ';').id < 0) return unexpected_token(*tokenizer);

        put(&stack[len(stack) - 1].variables, identifier, (Node) {
            .prototype = &Variable,
            .type = Box((Node) {
                .prototype = &FunctionType,
                .base_type = {
                    .declaration = Box((Node) {
                        .prototype = &FunctionDeclaration,
                        .function_declaration = {
                            .signature = signature,
                            .identifier = identifier,
                        },
                    }),
                }
            }),
            .variable = {
                .identifier = identifier,
            }
        });

        return (Node) { .prototype = &Ignore };
    }

    
    
    if(gimme(tokenizer, '{').id < 0) return unexpected_token(*tokenizer);

    Node declaration = {
        .prototype = &FunctionDeclaration,
        .function_declaration = {
            .signature = signature,
            .argument_names = argument_names,
            .identifier = identifier,
            .body = Box(Body(tokenizer, '}', 0)),
        },
    };

    put(&stack[len(stack) - 1].variables, identifier, (Node) {
        .prototype = &Variable,
        .type = Box((Node) {
            .prototype = &FunctionType,
            .base_type = {
                .declaration = Box(declaration),
            }
        }),
        .variable = {
            .identifier = identifier,
        },
    });

    return declaration;
}

#endif