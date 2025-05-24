#include "node.c"

int errc = 0;
void Error(Node node, FILE* file) {
    fprintf(file, "<error (%d)>", ++errc);
    fprintf(stderr, "(%d) \33[31mE:\33[0m ", errc);
    node.error.perror(node);
}

void UnexpectedToken(Node node) {
    fprintf(stderr, "Unexpected token \33[36m%.*s\33[0m\n",
        node.error.label.len, node.error.label.data);
}

void CannotFind(Node node) {
    fprintf(stderr, "Cannot find %s \33[36m%.*s\33[0m in scope\n",
        node.error.cannot_find, node.error.label.len, node.error.label.data);
}

void Constant(Node node, FILE* file) {
    switch(node.constant.type) {
        case cNumber: fprintf(file, "%ld", node.constant.number); break;
        case cString: fprintf(file, "%.*s",
            node.constant.string.len, node.constant.string.data); break;
    }
}

void CType(Node node, FILE* file) {
    fprintf(file, "%s", node.defined_type.c_type);
}

void FunctionType(Node node, FILE* file) {
    Node* signature = node.defined_type.declaration->function_declaration.signature;
    fprintf(file, "typeof(");
    signature->prototype(*signature, file);
    fprintf(file, "(*)())");
}

void NumberType(Node node, FILE* file) {
    fprintf(file, "int");
}

void StructureType(Node node, FILE* file) {
    str identifier = node.defined_type.declaration->structure_declaration.identifier;
    fprintf(file, "struct %.*s", identifier.len, identifier.data);
}

void PointerType(Node node, FILE* file) {
    node.defined_type.child->prototype(*node.defined_type.child, file);
    fprintf(file, "*");
}

void Variable(Node node, FILE* file) {
    fprintf(file, "%.*s",
        node.variable.identifier.len, node.variable.identifier.data);
}

void AutoError(Node node) {
    fprintf(stderr, "Could not infer type \33[36mauto\33[0m\n");
}

void Auto(Node node, FILE* file) {
    Error((Node) {
        .error = {
            .perror = &AutoError,
        }
    }, file);
}

void TypeMismatch(Node node) {
    fprintf(stderr, "Type mismatch between \33[36m");
    node.error.mismatch.a->prototype(*node.error.mismatch.a, stderr);
    fprintf(stderr, "\33[0m and \33[36m");
    node.error.mismatch.b->prototype(*node.error.mismatch.b, stderr);
    fprintf(stderr, "\33[0m\n");
}

void Structure(Node node, FILE* file) {
    str identifier = node.type->defined_type.declaration->structure_declaration.identifier;
    str* keys = node.structure.keys;
    Node* values = node.structure.values;

    fprintf(file, "(struct %.*s) { ", identifier.len, identifier.data);
    for(int i = 0; i < len(keys); i++) {
        fprintf(file, ".%.*s = ", keys[i].len, keys[i].data);
        values[i].prototype(values[i], file);
        fprintf(file, ", ");
    }
    fprintf(file, "}");
}

void StructureDeclaration(Node node, FILE* file) {
    str identifier = node.structure_declaration.identifier;
    str* keys = node.structure_declaration.keys;
    Node* types = node.structure_declaration.types;

    fprintf(file, "struct %.*s { ", identifier.len, identifier.data);
    for(int i = 0; i < len(keys); i++) {
        types[i].prototype(types[i], file);
        fprintf(file, " %.*s; ", keys[i].len, keys[i].data);
    }
    fprintf(file, "};\n");
}

int type_match(Node* modifiable, Node match) {
    if(modifiable->prototype == &Auto) {
        *modifiable = match;
        return 1;
    }

    if(modifiable->defined_type.meta & tIsNumeric
        && match.defined_type.meta & tIsNumeric) return 1;

    if(modifiable->prototype == &Error
    || modifiable->prototype != match.prototype) return 0;

    if(modifiable->prototype == &CType) {
        return !strcmp(modifiable->defined_type.c_type, match.defined_type.c_type);
    }

    if(modifiable->prototype == &FunctionType) {
        Node* modifiable_signature = modifiable->defined_type
            .declaration->function_declaration.signature;
        Node* match_signature = match.defined_type
            .declaration->function_declaration.signature;

        for(int i = 0; i < len(modifiable_signature); i++) {
            if(!type_match(&modifiable_signature[i], match_signature[i])) return 0;
        }

        return 1;
    }

    return 0;
}

Node number_type = {
    .prototype = &NumberType,
    .defined_type = {
        .meta = tIsNumeric,
    },
};
Node* error_type;

Node Literal(str* tokenizer) {
    str token = next(tokenizer);

    switch(token.id) {
        case 'n': return (Node) {
            .prototype = &Constant,
            .type = &number_type,
            .constant = {
                .type = cNumber,
                .number = strtol(token.data, 0, 0),
            },
        };

        case 'i': {
            for(int i = 0; i < len(var_names); i++) {
                if(streq(token, var_names[i])) return vars[i];
            }
            return (Node) {
                .prototype = &Error,
                .type = error_type,
                .error = {
                    .perror = &CannotFind,
                    .cannot_find = "variable",
                    .label = token,
                },
            };
        }

        case '"': {
            str* keys = 0;
            Node* values = 0;

            push(&keys, constr("data"));
            push(&values, (Node) {
                .prototype = &Constant,
                .constant = {
                    .type = cString,
                    .string = token,
                },
            });
            push(&keys, constr("size"));
            push(&values, (Node) {
                .prototype = &Constant,
                .constant = {
                    .type = cNumber,
                    .number = token.len - 2,
                },
            });

            return (Node) {
                .prototype = &Structure,
                .type = Box((Node) {
                    .prototype = &StructureType,
                    .defined_type = {
                        .declaration = Box((Node) {
                            .prototype = &StructureDeclaration,
                            .structure_declaration = {
                                .identifier = constr("str"),
                            },
                        }),
                    },
                }),
                .structure = {
                    .keys = keys,
                    .values = values,
                },
            };
        }
    }

    return (Node) {
        .prototype = &Error,
        .type = error_type,
        .error = {
            .perror = &UnexpectedToken,
            .label = token,
        },
    };
}

Node Type(str* tokenizer) {
    str token = next(tokenizer);

    switch(token.id) {
        case 'i': {
            for(int i = 0; i < len(type_names); i++) {
                if(streq(token, type_names[i])) return types[i];
            }

            return (Node) {
                .prototype = &Error,
                .type = error_type,
                .error = {
                    .perror = &CannotFind,
                    .cannot_find = "type",
                    .label = token,
                },
            };
        }

        case '&': {
            return (Node) {
                .prototype = &PointerType,
                .defined_type = {
                    .child = Box(Type(tokenizer)),
                },
            };
        }
    }

    return (Node) {
        .prototype = &Error,
        .type = error_type,
        .error = {
            .perror = &UnexpectedToken,
            .label = *tokenizer,
        },
    };
}