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
    Vec(Node*) arguments = node.function_call.arguments;

    fprintf(file, "%.*s(", function.variable.identifier.len, 
        function.variable.identifier.data);
    for(int i = 0; i < len(arguments); i++) {
        arguments[i]->prototype(*arguments[i], file);
        if(i != len(arguments) - 1) fprintf(file, ", ");
    }
    fprintf(file, ")");
}

void FunctionDeclaration(Node node, FILE* file) {
    Vec(Node*) signature = node.function_declaration.signature;
    Vec(str) argument_names = node.function_declaration.argument_names;
    str identifier = node.function_declaration.identifier;
    Node* body = node.function_declaration.body;

    signature[0]->prototype(*signature[0], file);
    fprintf(file, " %.*s(", identifier.len, identifier.data);

    for(int i = 0; i < len(argument_names); i++) {
        signature[i + 1]->prototype(*signature[i + 1], file); 
        fprintf(file, " %.*s%s", argument_names[i].len, argument_names[i].data,
            i >= len(argument_names) - 1 ? "" : ", ");
    }

    fprintf(file, ") {\n");
    body->prototype(*body, file);
    fprintf(file, "%.*s}\n", (int) len(indent) * 4, (char*)(void*) indent);
}

void GenericType(Node node, FILE* file) {
    Node* type = (*node.base_type.generic.generics)[node.base_type.generic.index];
    type->prototype(*type, file);
}

void organize_function_generics(str* tokenizer, Node* function) {
    Node* declaration = function->type->base_type.declaration;
    Vec(Node*) generics = declaration->function_declaration.generics;
    declaration->function_declaration.generics = 0;
    
    for(int i = 0; i < len(generics); i++) {
        push(&declaration->function_declaration.generics, Box((Node) { .prototype = &Auto }));
    }

    if(tokenizer->id == '<') {
        next(tokenizer);
        
        int generic_index = 0;
        // TODO: handle type arguments
        // modify declaration generics
        puts("not implemented");
        exit(1);
    }
}

Node* function_declarations;

__attribute__ ((constructor))
void _init_function_declarations() {
    function_declarations = Box((Node) { .prototype = &BodyPrototype });
}

Node void_type = {
    .prototype = &CType,
    .base_type = {
        .c_type = constr("void"),
    },
};

Node* parse_function_call(str* tokenizer, Node* left) {
    if(left->prototype != &Variable || left->type->prototype != &FunctionType)
        return type_mismatch(left, Box((Node) {
            .prototype = &FunctionType,
            .base_type = {
                .declaration = Box((Node) {
                    .prototype = &FunctionDeclaration,
                    .function_declaration = {
                        .signature = vec(&void_type),
                    },
                }),
            },
        }));

    next(tokenizer);
    Vec(Node*) arguments = 0;
    Vec(Node*) signature = left->type->base_type.declaration->function_declaration.signature;
    int signature_index = 1;

    while(tokenizer->id != ')') {
        if(signature_index >= len(signature))
            return type_mismatch(Expression(tokenizer, 100)->type, left->type);

        Node* argument = Expression(tokenizer, 100);
        if(!type_match(&signature[signature_index++], argument->type))
            push(&arguments, type_mismatch(signature[signature_index - 1], argument->type));
        else push(&arguments, argument);

        if(tokenizer->id == ',') next(tokenizer);
        else break;
    }
    if(gimme(tokenizer, ')').id < 0) return unexpected_token(*tokenizer);

    Vec(Node*) type_arguments = 0;
    for(int i = 0; i < len(left->type->base_type.declaration->function_declaration.generics); i++) {
        push(&type_arguments, 
            left->type->base_type.declaration->function_declaration.generics[i]);
    }

    if(left->type->base_type.declaration->function_declaration.meta & fGeneric
        && !(left->type->base_type.declaration->function_declaration.meta & fExternal)) {
        Map(int)* monomorphized_functions = &left->type->base_type
            .declaration->function_declaration.monomorphized_functions;
        str key_string = { 0, len(type_arguments) * sizeof(void*), (void*) type_arguments };
        Node* declaration = left->type->base_type.declaration;

        if(!get(*monomorphized_functions, key_string)) {
            put(monomorphized_functions, key_string, 1);

            push(&function_declarations->body.children, Box((Node) {
                .prototype = &GenericWrapper,
                .generic_wrapper = {
                    .child = declaration,
                    .generics = type_arguments,
                    .generics_override = &declaration->function_declaration.generics,
                    .identifier = declaration->function_declaration.identifier,
                    .identifier_overrides = vec(&declaration->function_declaration.identifier),
                }
            }));
        }

        Node* generic_wrapper = Box((Node) {
            .prototype = &GenericWrapper,
            .type = signature[0],
            .generic_wrapper = {
                .child = Box((Node) {
                    .prototype = &FunctionCall,
                    .type = Box((Node) {
                        .prototype = &GenericTypeWrapper,
                        .base_type = {
                            .generic_wrapper = {
                                .child = signature[0],
                            }
                        }
                    }),
                    .function_call = {
                        .function = left,
                        .arguments = arguments,
                    }
                }),
                .generics = type_arguments,
                .generics_override = &declaration->function_declaration.generics,
                .identifier = left->variable.identifier,
                .identifier_overrides = vec(&left->variable.identifier,
                    &declaration->function_declaration.identifier),
            }
        });

        generic_wrapper->generic_wrapper.child->type->base_type.generic_wrapper.parent = generic_wrapper;
        return generic_wrapper;
    }

    return Box((Node) {
        .prototype = &FunctionCall,
        .type = signature[0],
        .function_call = {
            .function = left,
            .arguments = arguments,
        }
    });
}

Node* parse_function_declaration(str* tokenizer) {
    next(tokenizer);

    str identifier = gimme(tokenizer, 'i');
    if(identifier.id < 0) return unexpected_token(*tokenizer);

    Node* declaration = Box((Node) {
        .prototype = &FunctionDeclaration,
        .function_declaration = {
            .identifier = stack[len(stack) - 1].namespace
                ? strc(stack[len(stack) - 1].namespace->namespace.identifier, constr("__"), identifier)
                : identifier,
            .signature = vec((Node*) 0),
        }
    });

    Node pre_body = { .prototype = &BodyPrototype };
    if(gimme(tokenizer, '<').id == '<') {
        declaration->function_declaration.meta |= fGeneric;

        while(tokenizer->id != '>') {
            str generic_name = gimme(tokenizer, 'i');
            if(generic_name.id < 0) return unexpected_token(*tokenizer);

            put(&pre_body.body.context.types, generic_name, Box((Node) {
                .prototype = &GenericType,
                .base_type = {
                    .generic = {
                        .generics = &declaration->function_declaration.generics,
                        .index = len(declaration->function_declaration.generics),
                    },
                },
            }));

            push(&declaration->function_declaration.generics, Box((Node) { .prototype = &Auto }));

            if(gimme(tokenizer, ',').id < 0) break;
        }
        if(gimme(tokenizer, '>').id < 0) return unexpected_token(*tokenizer);
    }
    push(&stack, pre_body.body.context);

    if(gimme(tokenizer, '(').id < 0) return unexpected_token(*tokenizer);
    while(tokenizer->id != ')') {
        str argument_name = gimme(tokenizer, 'i');
        if(argument_name.id < 0 || gimme(tokenizer, ':').id < 0)
            return unexpected_token(*tokenizer);
        Node* argument_type = Type(tokenizer);

        push(&declaration->function_declaration.signature, argument_type);
        push(&declaration->function_declaration.argument_names, argument_name);

        put(&pre_body.body.context.variables, argument_name, Box((Node) {
            .prototype = &Variable,
            .type = argument_type,
            .variable = {
                .identifier = argument_name,
            },
        }));

        if(gimme(tokenizer, ',').id < 0) break;
    }
    if(gimme(tokenizer, ')').id < 0) return unexpected_token(*tokenizer);

    if(tokenizer->id == 'r') {
        next(tokenizer);
        declaration->function_declaration.signature[0] = Type(tokenizer);
    } else {
        declaration->function_declaration.signature[0] = &void_type;
    }

    pop(stack);

    if(tokenizer->id == 'i' && streq(*tokenizer, constr("__external"))) {
        next(tokenizer);
        declaration->function_declaration.meta |= fExternal;

        if(gimme(tokenizer, 'O' /* :: */).id == 'O') {
            str c_identifier = gimme(tokenizer, 'i');
            if(c_identifier.id < 0) return unexpected_token(*tokenizer);
            declaration->function_declaration.identifier = c_identifier;
        }

        if(gimme(tokenizer, ';').id < 0) return unexpected_token(*tokenizer);

        put(&stack[len(stack) - 1].variables, identifier, Box((Node) {
            .prototype = &Variable,
            .type = Box((Node) {
                .prototype = &FunctionType,
                .base_type = {
                    .declaration = declaration,
                }
            }),
            .variable = {
                .identifier = declaration->function_declaration.identifier,
            }
        }));

        return Box((Node) { .prototype = &Ignore });
    }
    
    if(gimme(tokenizer, '{').id < 0) return unexpected_token(*tokenizer);
    declaration->function_declaration.body = Body(tokenizer, '}', 0, Box(pre_body));

    if(!(declaration->function_declaration.meta & fGeneric))
        push(&function_declarations->body.children, declaration); 

    put(&stack[len(stack) - 1].variables, identifier, Box((Node) {
        .prototype = &Variable,
        .type = Box((Node) {
            .prototype = &FunctionType,
            .base_type = {
                .declaration = declaration,
            }
        }),
        .variable = {
            .meta = stack[len(stack) - 1].namespace ? tHidden : 0,
            .identifier = declaration->function_declaration.identifier,
        },
    }));

    return Box((Node) { .prototype = &Ignore });
}

#endif