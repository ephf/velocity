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
    .type_meta = tIsNumeric | tAutoConst,
};

Node* parse_number_constant(str token) {
    return Box((Node) {
        .prototype = &Constant,
        .type = &numeric_type,
        .constant = {
            .type = cNumber,
            .number = strtol(token.data, 0, 0),
        },
    });
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

Node* parse_string_constant(str token) {
    return Box((Node) {
        .prototype = &Structure,
        .type = &str_type,
        .structure = {
            .body = map(str, Node*, {
                { constr("data"), Box((Node) {
                    .prototype = &Constant,
                    .constant = {
                        .type = cString,
                        .string = token,
                    },
                }) },
                { constr("size"), Box((Node) {
                    .prototype = &Constant,
                    .constant = {
                        .type = cNumber,
                        .number = token.len - 2,
                    },
                }) },
            }),
        },
    });
}

#endif