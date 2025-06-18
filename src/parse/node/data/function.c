#ifndef FUNCTION_C
#define FUNCTION_C

#include "../basic.c"
#include "variable.c"
#include "../../include.h"

#include "../symbol/identifier.c"
#include "../symbol/generics.c"

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
        fprintf(file, " %.*s%s", hoisted ? 0 : argument_names[i].len, argument_names[i].data,
            i >= len(argument_names) - 1 ? "" : ", ");
    }

    fprintf(file, ")");
    if(hoisted) {
        fprintf(file, ";\n");
        return;
    }

    fprintf(file, " {\n");
    body->prototype(*body, file);
    fprintf(file, "%.*s}\n", (int) len(indent) * 4, (char*)(void*) indent);
}

Node* parse_function_call(Tokenizer* tokenizer, Node* left) {
    if(left->prototype != &Variable || left->type->prototype != &FunctionType) {
        push(&errors, type_mismatch(left->range, left, Box((Node) { &FunctionType, .base_type = {
            .declaration = Box((Node) { &FunctionDeclaration, .function_declaration = {
                .signature = vec(&void_type)
            }}),
        }})));
        return Box((Node) { &Ignore });
    }

    Node* declaration = left->type->base_type.declaration;
    reset_generics(declaration);

    if(try_token(tokenizer, '<', 0)) resolve_type_arguments(tokenizer, &declaration->declaration_generics);
    expect_token(tokenizer, '(');

    Vec(Node*) arguments = 0;
    for(int i = 0; i < len(left->variable.bounded_function_arguments); i++)
        push(&arguments, left->variable.bounded_function_arguments[i]);

    Vec(Node*) signature = declaration->function_declaration.signature;
    int signature_index = 1 + len(left->variable.bounded_function_arguments);

    while(tokenizer->current.type != ')') {
        if(signature_index >= len(signature)) {
            Node* expression = Expression(tokenizer, 100);
            push(&errors, type_mismatch(expression->range, expression->type, &void_type));
            continue;
        }

        Node* argument = Expression(tokenizer, 100);
        if(!type_match(&signature[signature_index++], argument->type)) {
            Error mismatch = type_mismatch(argument->range, argument->type, signature[signature_index - 1]);
            push(&mismatch.attachments, create_see_error((Error) {
                .fault_point = signature[signature_index - 1]->range,
                .perror = &SeeDeclaration,
            }));
            push(&errors, mismatch);
        } else push(&arguments, argument);

        if(!try_token(tokenizer, ',', 0) && tokenizer->current.type != ')') expect_token(tokenizer, ',');
    }
    TokenResult range_end = expect_token(tokenizer, ')');

    if(declaration->attributes & aGeneric
        && !(declaration->attributes & aExternal)) {
        
        Vec(Node*) wrapped_children = generic_wrap_declaration(tokenizer,
            (struct GenericWrapper) {
                .generics = declaration->declaration_generics,
                .generics_override = &declaration->declaration_generics,
                .identifier = declaration->function_declaration.identifier,
                .identifier_overrides = vec(
                    &left->variable.identifier,
                    &declaration->function_declaration.identifier),
            }, declaration,
            vec(signature[0], Box((Node) { &FunctionCall, 0, stretch_token(left->range, range_end.token),
                .type = signature[0], .function_call = {
                    .function = left,
                    .arguments = arguments,
                }
            })));

        wrapped_children[1]->type = wrapped_children[0];
        return wrapped_children[1];
    }

    return Box((Node) { &FunctionCall, 0, stretch_token(left->range, range_end.token),
        .type = signature[0],
        .function_call = {
            .function = left,
            .arguments = arguments,
        }
    });
}

Node* parse_function_declaration(Tokenizer* tokenizer) {
    TokenResult range_start = next_token(tokenizer);
    ResolvedIdentifier ri = resolve_identifier(tokenizer, 1);

    Node* declaration = Box((Node) { &FunctionDeclaration, 0, ri.base, .function_declaration = {
        .identifier = ri.constructed,
        .signature = vec((Node*) 0),
    }});

    Node* pre_body = Box((Node) { &BodyPrototype });

    if(try_token(tokenizer, '<', 0)) declaration->declaration_sub_generics = 
        collect_generics(tokenizer, &declaration->attributes, &pre_body->body.context,
            &declaration->declaration_generics);

    push(&stack, &pre_body->body.context);

    expect_token(tokenizer, '(');
    while(tokenizer->current.type != ')') {
        Token self_pointer = { .type = 0 };
        if(try_token(tokenizer, '&', &self_pointer) && !streq(tokenizer->current.str, constr("self"))) {
            push(&errors, expected_keyword(next_token(tokenizer).token, "self"));
            continue;
        }

        TokenResult argument_name = expect_token(tokenizer, TokenIdentifier);

        Node* argument_type = 0;
        if(tokenizer->current.type != ':' && streq(argument_name.token.str, constr("self"))) {
            if(!ri.scope->namespace || ri.scope->namespace->namespace.parent_type) {
                push(&errors, cannot_find(argument_name.token, "parent structure"));
                continue;
            }

            argument_type = ri.scope->namespace->namespace.parent_type;

            if(self_pointer.type == '&') argument_type = Box((Node) { &ModifiedType, 0,
                stretch_token(self_pointer, argument_name.token), .modified_type = {
                    .base = argument_type,
                    .modifiers = vec(mPointer),
                }
            });
        } else expect_token(tokenizer, ':');
        if(!argument_type) argument_type = Type(tokenizer);

        push(&declaration->function_declaration.signature, argument_type);
        push(&declaration->function_declaration.argument_names, argument_name.token.str);

        put(&pre_body->body.context.variables, argument_name.token.str, Box((Node) { &Variable, 0,
            argument_name.token, .type = argument_type, .variable = {
                .identifier = argument_name.token.str,
            },
        }));

        if(!try_token(tokenizer, ',', 0) && tokenizer->current.type != ')') expect_token(tokenizer, ',');
    }
    TokenResult range_end = expect_token(tokenizer, ')');
    declaration->range = stretch_token(range_start.token, range_end.token);

    declaration->function_declaration.signature[0] = try_token(tokenizer, TokenRightArrow, 0)
        ? Type(tokenizer) : &void_type;
    pop(stack);

    if(streq(tokenizer->current.str, constr("external"))) {
        next_token(tokenizer);
        declaration->attributes |= aExternal;
        if(declaration->attributes & aGeneric) pop(generics_stack);

        Token external_identifier;
        if(try_token(tokenizer, TokenString, &external_identifier))
            declaration->function_declaration.identifier = (str) {
                external_identifier.str.len - 2, external_identifier.str.data + 1 };
        else if(try_token(tokenizer, TokenIdentifier, &external_identifier))
            declaration->function_declaration.identifier = external_identifier.str;

        expect_token(tokenizer, ';');

        put(&ri.scope->variables, ri.base.str, Box((Node) { &Variable, 0, ri.base,
            .type = Box((Node) { &FunctionType, 0, declaration->range,
                .base_type = { .declaration = declaration }
            }),
            .variable = { declaration->function_declaration.identifier }
        }));

        return Box((Node) { .prototype = &Ignore });
    }
    
    expect_token(tokenizer, '{');
    declaration->function_declaration.body = Body(tokenizer, '}', 0, pre_body);
    if(declaration->attributes & aGeneric) pop(generics_stack);

    put(&ri.scope->variables, ri.base.str, Box((Node) { &Variable, 0, ri.base,
        .type = Box((Node) { &FunctionType, .base_type = { .declaration = declaration } }),
        .variable = { ri.constructed },
    }));

    if(!(declaration->attributes & aGeneric)) {
        push(&hoist_section->body.children, declaration);
        return declaration;
    } else return Box((Node) { &Ignore });
}

#endif