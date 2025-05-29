#include "node.c"

#include "node/data/function.c"

void ModifiedType(Node node, FILE* file) {
    fprintf(file, "typeof(");
    node.modified_type.base->prototype(*node.modified_type.base, file);

    for(int i = len(node.modified_type.variadic_arguments); i--;)
        fprintf(file, node.modified_type.modifiers[i] == mPointer ? "*" : "(*");
    for(int i = 0; i < len(node.modified_type.variadic_arguments); i++)
        if(node.modified_type.modifiers[i] == mArray) fprintf(file, ")[]");
    fprintf(file, ")");
}

int type_match(Node** modifiable, Node* match) {
    if((*modifiable)->prototype == &Auto) {
        if((*modifiable)->type_meta & tIsNumeric
            && !(match->type_meta & tIsNumeric)) return 0;

        *modifiable = match;
        return 1;
    }

    if((*modifiable)->prototype == &Error
        || match->prototype == &Error) return 0;

    if((*modifiable)->type_meta & tIsNumeric
        && match->type_meta & tIsNumeric) return 1;

    if((*modifiable)->prototype == &ModifiedType || match->prototype == &ModifiedType) {
        if((*modifiable)->prototype != match->prototype) return 0;

        if(len((*modifiable)->modified_type.variadic_arguments) !=
            len(match->modified_type.variadic_arguments)) return 0;
        for(int i = 0; i < len((*modifiable)->modified_type.variadic_arguments); i++)
            if((*modifiable)->modified_type.modifiers[i] !=
                match->modified_type.modifiers[i]) return 0;

        return type_match(&(*modifiable)->modified_type.base, match->modified_type.base);
    }

    if((*modifiable)->prototype == &CType)
        return !strcmp((*modifiable)->base_type.c_type, match->base_type.c_type);

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
                Node* type = get(stack[i].types, token);
                if(type) return Box((Node) {
                    .prototype = &ModifiedType,
                    .modified_type = {
                        .base = type,
                    },
                });
            }

            return Box(cannot_find(token, "type"));
        }

        case '&': {
            Node* type = Type(tokenizer);
            push(&type->modified_type.modifiers, mPointer);
            type->type_meta = tIsNumeric;
            return type;
        }
    }

    return Box(unexpected_token(token));
}

#undef ctype