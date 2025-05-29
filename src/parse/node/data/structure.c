#ifndef STRUCTURE_DECLARATION_C
#define STRUCTURE_DECLARATION_C

#include "../basic.c"

void Structure(Node node, FILE* file) {
    str identifier = node.type->base_type.declaration->structure_declaration.identifier;
    Map(Node) body = node.structure.body;

    fprintf(file, "(struct %.*s) { ", identifier.len, identifier.data);
    entries(body, str key, Node value) {
        fprintf(file, ".%.*s = ", key.len, key.data);
        value.prototype(value, file);
        fprintf(file, ", ");
    }
    fprintf(file, "}");
}

void StructureType(Node node, FILE* file) {
    str identifier = node.base_type.declaration->structure_declaration.identifier;
    fprintf(file, "struct %.*s", identifier.len, identifier.data);
}

void StructureDeclaration(Node node, FILE* file) {
    str identifier = node.structure_declaration.identifier;
    Map(Node*) body = node.structure_declaration.body;

    fprintf(file, "struct %.*s { ", identifier.len, identifier.data);
    entries(body, str key, Node* type) {
        type->prototype(*type, file);
        fprintf(file, " %.*s; ", key.len, key.data);
    }
    fprintf(file, "};\n");
}

Node parse_structure_declaration(str* tokenizer) {
    next(tokenizer);

    str identifier = gimme(tokenizer, 'i');
    if(identifier.id < 0 || gimme(tokenizer, '{').id < 0)
        return unexpected_token(*tokenizer);

    Map(Node*) body = 0;

    while(tokenizer->id > 0 && tokenizer->id != '}') {
        str key = gimme(tokenizer, 'i');
        if(key.id < 0) return unexpected_token(*tokenizer);

        if(tokenizer->id == ':') {
            next(tokenizer);
            put(&body, key, Type(tokenizer));
        } else {
            put(&body, key, Box((Node) { .prototype = &Auto }));
        }

        if(tokenizer->id == ',') next(tokenizer);
        else break;
    }
    if(gimme(tokenizer, '}').id < 0) return unexpected_token(*tokenizer);

    Node declaration = {
        .prototype = &StructureDeclaration,
        .structure_declaration = {
            .identifier = identifier,
            .body = body,
        },
    };

    put(&stack[len(stack) - 1].types, identifier, (Node) {
        .prototype = &Structure,
        .base_type = {
            .declaration = Box(declaration),
        },
    });

    return declaration;
}

#endif