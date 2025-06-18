#include "node.c"

#include "node/data/function.c"
#include "node/data/structure.c"

void ModifiedType(Node node, FILE* file) {
    fprintf(file, "typeof(");
    node.modified_type.base->prototype(*node.modified_type.base, file);

    for(int i = len(node.modified_type.modifiers); i--;)
        fprintf(file, node.modified_type.modifiers[i] == mPointer ? "*" : "(*");
    for(int i = 0; i < len(node.modified_type.modifiers); i++)
        if(node.modified_type.modifiers[i] == mArray) fprintf(file, ")[]");
    fprintf(file, ")");
}

str stringify_type(Node* type) {
    if(type->prototype == &CType)
        return type->base_type.c_type;
    
    if(type->prototype == &StructureType)
        return type->base_type.declaration->structure_declaration.identifier;

    if(type->prototype == &FunctionType) {
        str base = strc(constr("fn__"), type->base_type.declaration->function_declaration.identifier);
        Vec(Node*) generics = type->base_type.declaration->declaration_generics;
        for(int i = 0; i < len(generics); i++)
            base = strc(base, constr("_"), stringify_type(generics[i]));
        return base;
    }

    if(type->prototype == &ModifiedType) {
        str base = stringify_type(type->modified_type.base);
        for(int i = 0; i < len(type->modified_type.modifiers); i++)
            base = strc(base, type->modified_type.modifiers[i] == mPointer
                ? constr("ptr") : constr("array"));
        return base;
    }

    if(type->prototype == &Auto)
        return constr("int");
    
    if(type->prototype == &GenericType)
        return stringify_type((*type->base_type.generic.generics)[type->base_type.generic.index]);

    return constr("<stringify type unimplemented>");
}

void TypeMismatch(Error error) {
    str a = stringify_type(error.as_type_mismatch.a);
    str b = stringify_type(error.as_type_mismatch.b);

    fprintf(stderr, "type mismatch between \33[33m%.*s\33[0m and \33[33m%.*s\33[0m",
        a.len, a.data, b.len, b.data);
}

Error type_mismatch(Token range, Node* a, Node* b) {
    return create_basic_error((Error) {
        .fault_point = range,
        .perror = &TypeMismatch,
        .as_type_mismatch = { a, b },
    });
}

int type_match(Node** modifiable, Node* match) {
    while((*modifiable)->prototype == &GenericType)
        modifiable = &(*(*modifiable)->base_type.generic.generics)[(*modifiable)->base_type.generic.index];
    while(match->prototype == &GenericType)
            match = (*match->base_type.generic.generics)[match->base_type.generic.index];

    if((*modifiable)->prototype == &GenericWrapper) {
        ConformGenerics(*modifiable);
        return type_match(&(*modifiable)->generic_wrapper.child, match);
    }
    if(match->prototype == &GenericWrapper) {
        ConformGenerics(match);
        return type_match(modifiable, match->generic_wrapper.child);
    }

    if((*modifiable)->prototype == &Auto) {
        if((*modifiable)->attributes & aNumeric && !(match->attributes & aNumeric)) return 0;
        *modifiable = match;
        return 1;
    }

    if((*modifiable)->attributes & (aNumeric | aPointer) && match->attributes & (aNumeric | aPointer))
        return 1;

    if((*modifiable)->prototype == &ModifiedType || match->prototype == &ModifiedType) {
        if((*modifiable)->prototype != match->prototype) return 0;

        if(len((*modifiable)->modified_type.modifiers) !=
            len(match->modified_type.modifiers)) return 0;
        for(int i = 0; i < len((*modifiable)->modified_type.modifiers); i++)
            if((*modifiable)->modified_type.modifiers[i] !=
                match->modified_type.modifiers[i]) return 0;

        return type_match(&(*modifiable)->modified_type.base, match->modified_type.base);
    }

    if((*modifiable)->prototype == &CType && match->prototype == &CType)
        return streq((*modifiable)->base_type.c_type, match->base_type.c_type);

    if((*modifiable)->prototype == &FunctionType && match->prototype == &FunctionType) {
        Vec(Node*) modifiable_signature = (*modifiable)->base_type
            .declaration->function_declaration.signature;
        Vec(Node*) match_signature = match->base_type
            .declaration->function_declaration.signature;

        for(int i = 0; i < len(modifiable_signature); i++)
            if(!type_match(&modifiable_signature[i], match_signature[i])) return 0;

        return 1;
    }

    if((*modifiable)->prototype == &StructureType && match->prototype == &StructureType) {
        return streq((*modifiable)->base_type.declaration->structure_declaration.identifier,
            match->base_type.declaration->structure_declaration.identifier);
    }

    return 0;
}

Node* dereference_type(Token range, Node* type) {
    if(type->prototype != &ModifiedType
        || type->modified_type.modifiers[len(type->modified_type.modifiers) - 1] != mPointer) {

        push(&errors, type_mismatch(range, type, Box((Node) { &ModifiedType, .modified_type = {
            .base = &void_type,
            .modifiers = vec(mPointer),
        }})));
        return type;
    }

    Vec(int) modifiers_clone = 0;
    if(len(type->modified_type.modifiers) >= 2) for(int i = 0; i < len(type->modified_type.modifiers) - 2; i++)
        push(&modifiers_clone, type->modified_type.modifiers[i]);
    
    int attributes = type->attributes & ~aPointer;
    if(modifiers_clone[len(modifiers_clone) - 1] == mPointer) attributes |= aPointer;
    
    return len(modifiers_clone)
        ? Box((Node) { &ModifiedType, attributes, range, .modified_type = {
            .base = type->modified_type.base,
            .modifiers = modifiers_clone,
        }})
        : type->modified_type.base;
}

Node* reference_type(Token range, Node* type) {
    Vec(int) modifiers_clone = 0;
    for(int i = 0; i < len(type->modified_type.modifiers); i++)
        push(&modifiers_clone, type->modified_type.modifiers[i]);
    push(&modifiers_clone, mPointer);

    return Box((Node) { &ModifiedType, type->attributes | aPointer, range, .modified_type = {
        .base = type->prototype == &ModifiedType ? type->modified_type.base : type,
        .modifiers = modifiers_clone,
    }});
}

Node* Type(Tokenizer*);

Node* type_base(Tokenizer* tokenizer) {
    TokenResult base = next_token(tokenizer);

    switch(base.token.type) {
        case TokenIdentifier: {
            if(streq(base.token.str, constr("external"))) {
                Node* type = Box((Node) { &CType });

                if(streq(tokenizer->current.str, constr("numeric"))) {
                    next_token(tokenizer);
                    type->attributes |= aNumeric;
                }

                Token c_type;
                if(!try_token(tokenizer, TokenString, &c_type))
                    c_type = expect_token(tokenizer, TokenIdentifier).token;
                
                type->base_type.c_type = c_type.type == TokenIdentifier
                    ? c_type.str : (str) { .len = c_type.str.len - 2, .data = c_type.str.data + 1 };
                return type;
            }

            ResolvedIdentifier ri = resolve_identifier_given(base.token, tokenizer, 0);

            Node* type = 0;
            if(ri.scope) {
                Node** type_search = get(ri.scope->types, ri.base.str);
                type = type_search ? *type_search : 0;
            } else {
                for(int i = len(stack); i--; ) {
                    Node** type_search = get(stack[i]->types, ri.base.str);
                    if(type_search) {
                        type = *type_search;
                        break;
                    }
                }
            }

            if(!type) {
                push(&errors, cannot_find(ri.base, "type"));
                return Box((Node) { &Ignore, 0, ri.base, .type = &void_type });
            }

            return type;
        }

        case '&':
        case TokenDoubleAmp: {
            Node* type = Type(tokenizer);

            Node* ref = reference_type(type->range, type);
            if(base.token.type == TokenDoubleAmp) ref = reference_type(type->range, ref);

            return ref;
        }
    }

    push(&errors, unexpected_token(base.token, "i forgot to implement this shit correctly part 2"));
    return Box((Node) { &Ignore, 0, base.token, .type = &void_type });
}

Node* Type(Tokenizer* tokenizer) {
    Node* base = type_base(tokenizer);

    if(base->prototype == &StructureType
    && base->base_type.declaration->attributes & aGeneric && try_token(tokenizer, '<', 0)) {
        Node* declaration = base->base_type.declaration;
        reset_generics(declaration);
        resolve_type_arguments(tokenizer, &declaration->declaration_generics);

        return generic_wrap_declaration(tokenizer,
            (struct GenericWrapper) {
                .generics = declaration->declaration_generics,
                .generics_override = &declaration->declaration_generics,
                .identifier = declaration->structure_declaration.identifier,
                .identifier_overrides = vec(&declaration->structure_declaration.identifier),
            }, declaration, vec(base))[0]; 
    }

    return base;
}

#undef ctype