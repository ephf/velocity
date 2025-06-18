#ifndef STRUCTURE_DECLARATION_C
#define STRUCTURE_DECLARATION_C

#include "../basic.c"
#include "../symbol/identifier.c"
#include "../symbol/generics.c"

void Structure(Node node, FILE* file) {
    str identifier = node.type->base_type.declaration->structure_declaration.identifier;
    Map(Node*) body = node.structure.body;

    fprintf(file, "(struct %.*s) { ", identifier.len, identifier.data);
    entries(body, str key, Node* value) {
        fprintf(file, ".%.*s = ", key.len, key.data);
        value->prototype(*value, file);
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

    fprintf(file, "struct %.*s", identifier.len, identifier.data);
    if(hoisted) {
        fprintf(file, ";\n");
        return;
    }

    fprintf(file, " { ");
    entries(body, str key, Node* type) {
        type->prototype(*type, file);
        fprintf(file, " %.*s; ", key.len, key.data);
    }
    fprintf(file, "};\n");
}

Node* parse_structure_declaration(Tokenizer* tokenizer) {
    TokenResult range_start = next_token(tokenizer);
    ResolvedIdentifier ri = resolve_identifier(tokenizer, 1);

    Node* namespace = Box((Node) { &Namespace, 0, ri.base, .type = Box((Node) { &Auto }), .namespace = {
        .identifier = ri.constructed,
        .body = Box((Node) { &BodyPrototype }),
        .parent_type = Box((Node) { &StructureType }),
    }});

    namespace->namespace.body->body.context = (Context) { .namespace = namespace };
    put(&ri.scope->variables, ri.base.str, namespace);

    Node* declaration = Box((Node) { &StructureDeclaration, 0, ri.base, .structure_declaration = {
        .identifier = ri.constructed,
        .namespace = namespace,
    }});
    namespace->namespace.parent_type->base_type.declaration = declaration;

    Context* generic_context = Box((Context) { 0 });
    push(&stack, generic_context);

    if(try_token(tokenizer, '<', 0)) declaration->declaration_sub_generics =
        collect_generics(tokenizer, &declaration->attributes, generic_context,
            &declaration->declaration_generics);

    declaration->range = namespace->range = stretch_token(range_start.token, ri.base);

    expect_token(tokenizer, '{');
    while(tokenizer->current.type != '}') {
        TokenResult key = expect_token(tokenizer, TokenIdentifier);
        if(key.is_error || expect_token(tokenizer, ':').is_error) continue;

        put(&declaration->structure_declaration.body, key.token.str, Type(tokenizer));
        if(!try_token(tokenizer, ',', 0) && tokenizer->current.type != '}') expect_token(tokenizer, ',');
    }
    expect_token(tokenizer, '}');
    pop(stack);
    if(declaration->attributes & aGeneric) pop(generics_stack);
    
    put(&ri.scope->types, ri.base.str, namespace->namespace.parent_type);

    if(!(declaration->attributes & aGeneric))
        push(&hoist_section->body.children, declaration);

    if(declaration->attributes & aGeneric) return Box((Node) { &Ignore });
    else return declaration;
}

#endif