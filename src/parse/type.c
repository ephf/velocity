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
        Vec(Node*) generics = type->base_type.declaration->function_declaration.generics;
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

    if(type->prototype == &Auto) {
        if(type->type_meta & tIsNumeric) return constr("int");
        return constr("AUTO");
    }

    return constr("UNKNOWN");
}

int type_match(Node** modifiable, Node* match) {
    while((*modifiable)->prototype == &GenericType)
        modifiable = &(*(*modifiable)->base_type.generic.generics)[(*modifiable)->base_type.generic.index];
    while(match->prototype == &GenericType)
            match = (*match->base_type.generic.generics)[match->base_type.generic.index];

    if((*modifiable)->prototype == &Error
        || match->prototype == &Error) return 0;

    if((*modifiable)->prototype == &Auto) {
        if((*modifiable)->type_meta & tIsNumeric
            && !(match->type_meta & tIsNumeric)) return 0;

        *modifiable = match;
        return 1;
    }

    if((*modifiable)->type_meta & tIsNumeric
        && match->type_meta & tIsNumeric) return 1;

    if((*modifiable)->prototype == &ModifiedType || match->prototype == &ModifiedType) {
        if((*modifiable)->prototype != match->prototype) return 0;

        if(len((*modifiable)->modified_type.modifiers) !=
            len(match->modified_type.modifiers)) return 0;
        for(int i = 0; i < len((*modifiable)->modified_type.modifiers); i++)
            if((*modifiable)->modified_type.modifiers[i] !=
                match->modified_type.modifiers[i]) return 0;

        return type_match(&(*modifiable)->modified_type.base, match->modified_type.base);
    }

    if((*modifiable)->prototype == &CType)
        return streq((*modifiable)->base_type.c_type, match->base_type.c_type);

    if((*modifiable)->prototype == &FunctionType) {
        Vec(Node*) modifiable_signature = (*modifiable)->base_type
            .declaration->function_declaration.signature;
        Vec(Node*) match_signature = match->base_type
            .declaration->function_declaration.signature;

        for(int i = 0; i < len(modifiable_signature); i++)
            if(!type_match(&modifiable_signature[i], match_signature[i])) return 0;

        return 1;
    }

    return 0;
}

Node* Type(str* tokenizer) {
    str token = next(tokenizer);

    switch(token.id) {
        case 'i': {
            for(int i = len(stack); i--; ) {
                Node** type = get(stack[i].types, token);
                if(type && !((*type)->type_meta & tHidden)) return *type;
            }

            return cannot_find(token, "type");
        }

        case '#':
        case '@': {
            str c_type = gimme(tokenizer, '"');
            if(c_type.id < 0) return unexpected_token(*tokenizer);
            return Box((Node) {
                .prototype = &CType,
                .type_meta = token.id == '#' ? tIsNumeric : 0,
                .base_type = {
                    .c_type = { .len = c_type.len - 2, .data = c_type.data + 1 },
                },
            });
        }

        case '&': {
            Node* type = Type(tokenizer);
            if(type->prototype != &ModifiedType) type = Box((Node) {
                .prototype = &ModifiedType,
                .modified_type = {
                    .base = type,
                },
            });

            push(&type->modified_type.modifiers, mPointer);
            type->type_meta = tIsNumeric;
            return type;
        }
    }

    return unexpected_token(token);
}

#undef ctype