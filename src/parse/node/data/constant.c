#ifndef CONSTANT_C
#define CONSTANT_C

#include "../basic.c"
#include "structure.c"

void Constant(Node node, FILE* file) {
    switch(node.constant.type) {
        case cNumber: fprintf(file, "%ld", node.constant.number); break;
        case cString: fprintf(file, "%.*s",
            node.constant.string.len, node.constant.string.data); break;
    }
}

Node numeric_type = {
    .prototype = &Auto,
    .attributes = aNumeric | aExternal,
};

Node* parse_number_constant(Token token) {
    return Box((Node) { &Constant, 0, token, .type = &numeric_type, .constant = {
        .type = cNumber,
        .number = strtol(token.str.data, 0, 0),
    }});
}

Node str_declaration = {
    .prototype = &StructureDeclaration,
    .structure_declaration = {
        .identifier = constr("str"),
    },
};

Node str_type = {
    .prototype = &StructureType,
    .base_type = {
        .declaration = &str_declaration,
    },
};

Node* parse_string_constant(Token token) {
    return Box((Node) { &Structure, 0, token, .type = &str_type, .structure = {
        .body = map(str, Node*, {
            { constr("data"), Box((Node) {
                .prototype = &Constant,
                .constant = {
                    .type = cString,
                    .string = token.str,
                },
            }) },
            { constr("size"), Box((Node) {
                .prototype = &Constant,
                .constant = {
                    .type = cNumber,
                    .number = token.str.len - 2,
                },
            }) },
        }),
    }});
}

#endif