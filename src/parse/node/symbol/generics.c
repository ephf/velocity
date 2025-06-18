#ifndef SYMBOL_GENERICS_C
#define SYMBOL_GENERICS_C

#include "../basic.c"

Vec(struct GenericWrapper)* collect_generics(Tokenizer* tokenizer, int* attributes, Context* context, Vec(Node*)* generics) {
    *attributes |= aGeneric;

    while(tokenizer->current.type != '>') {
        TokenResult identifier = expect_token(tokenizer, TokenIdentifier);
        if(identifier.is_error) continue;

        put(&context->types, identifier.token.str, Box((Node) { &GenericType, .base_type = {
            .generic = { generics, len(*generics) }
        }}));
        push(generics, Box((Node) { &Auto }));

        if(!try_token(tokenizer, ',', 0) && tokenizer->current.type != '>') expect_token(tokenizer, ',');
    }
    expect_token(tokenizer, '>');

    
    push(&generics_stack, Box((Vec(struct GenericWrapper)) 0));
    return generics_stack[len(generics_stack) - 1];
}

Token resolve_type_arguments(Tokenizer* tokenizer, Vec(Node*)* generics) {
    int index = 0;
    while(tokenizer->current.type != '>') {
        if(index >= len(*generics)) {
            Node* type = Type(tokenizer);
            push(&errors, type_mismatch(type->range, type, &void_type));
            continue;
        }

        (*generics)[index++] = Type(tokenizer);

        if(!try_token(tokenizer, ',', 0) && tokenizer->current.type != '>') expect_token(tokenizer, ',');
    }

    return expect_token(tokenizer, '>').token;
}

void post_generic_declaration(struct GenericWrapper wrapper) {
    str key_string = stringify_generics(wrapper.generics);
    if(!get(wrapper.child->declaration_map, key_string)) {
        put(&wrapper.child->declaration_map, key_string, 1);

        Node* wrapper_node = Box((Node) { &GenericWrapper, 0, wrapper.child->range, .generic_wrapper = wrapper });
        push(&hoist_section->body.children, wrapper_node);
        push(&generic_section->body.children, wrapper_node);
    }

    for(int i = 0; i < len(*wrapper.child->declaration_sub_generics); i++)
        post_generic_declaration((*wrapper.child->declaration_sub_generics)[i]);
}

bool incomplete_generic(Node* generic) {
    if(generic->prototype == &GenericType)
        return incomplete_generic((*generic->base_type.generic.generics)[generic->base_type.generic.index]);
    return generic->prototype == &Auto && !(generic->attributes & aExternal);
}

Vec(Node*) generic_wrap_declaration(Tokenizer* tokenizer, struct GenericWrapper wrapper,
    Node* declaration, Vec(Node*) children) {

    bool incomplete_generics = false;
    for(int i = 0; i < len(wrapper.generics); i++) {
        if(incomplete_generic(wrapper.generics[i])) {
            incomplete_generics = true;
            break;
        }
    }

    wrapper.child = declaration;
    if(incomplete_generics) {
        if(len(generics_stack) == 0) {
            Error error = expected_keyword(tokenizer->current, "completed generic");
            push(&error.attachments, create_see_error((Error) {
                .fault_point = declaration->range,
                .perror = &SeeDeclaration,
            }));
            push(&errors, error);
        }
        else push(generics_stack[len(generics_stack) - 1], wrapper);
    }
    else post_generic_declaration(wrapper);

    Vec(Node*) override_children = 0;
    for(int i = 0; i < len(children); i++) {
        wrapper.child = children[i];
        push(&override_children, Box((Node) { &GenericWrapper, children[i]->attributes, children[i]->range,
            .type = children[i]->type,
            .generic_wrapper = wrapper,
        }));
    }

    return override_children;
}

void reset_generics(Node* declaration) {
    Vec(Node*) old_generics = declaration->declaration_generics;
    declaration->declaration_generics = 0;

    for(int i = 0; i < len(old_generics); i++)
        push(&declaration->declaration_generics, Box((Node) { .prototype = &Auto }));
}

#endif