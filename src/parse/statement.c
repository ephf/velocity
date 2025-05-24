#include "postfix.c"

int initial_indent = 0;
void BodyPrototype(Node node, FILE* file) {
    if(initial_indent++) push(&indent, "    ");
    for(int i = 0; i < len(node.children); i++) {
        fprintf(file, "%.*s", (int) len(indent) * 4, (char*)(void*) indent);
        Node child = node.children[i];
        child.prototype(child, file);
    }
    if(--initial_indent) pop(indent);
}

void FunctionDeclaration(Node node, FILE* file) {
    Node* signature = node.function_declaration.signature;
    str identifier = node.function_declaration.identifier;
    Node* body = node.function_declaration.body;
    
    signature->prototype(*signature, file);
    fprintf(file, " %.*s() {\n", identifier.len, identifier.data);
    body->prototype(*body, file);
    fprintf(file, "%.*s}\n", (int) len(indent) * 4, (char*)(void*) indent);
}

void VariableDeclaration(Node node, FILE* file) {
    Node type = *node.variable_declaration.type;
    str identifier = node.variable_declaration.identifier;
    Node* value = node.variable_declaration.value;

    type.prototype(type, file);
    fprintf(file, " %.*s", identifier.len, identifier.data);
    if(value) {
        fprintf(file, " = ");
        value->prototype(*value, file);
    }
    fprintf(file, ";\n");
}

void StatementWrapper(Node node, FILE* file) {
    node.child->prototype(*node.child, file);
    fprintf(file, ";\n");
}

void ReturnStatement(Node node, FILE* file) {
    Node value = *node.return_statement.value;

    fprintf(file, "return ");
    value.prototype(value, file);
    fprintf(file, ";\n");
}

Node Body(str* tokenizer, int terminator);

Node Statement(str* tokenizer) {
    if(tokenizer->id == 'i') {
        if(streq(*tokenizer, constr("fn"))) {
            next(tokenizer);

            str identifier = gimme(tokenizer, 'i');
            if(identifier.id < 0 || gimme(tokenizer, '(').id < 0) return (Node) {
                .prototype = &Error,
                .error = {
                    .perror = &UnexpectedToken,
                    .label = identifier,
                },
            };

            Node* signature = 0;
            str* argument_names = 0;
            push(&signature, (Node) {
                .prototype = &Auto,
            });

            while(tokenizer->id != ')') {
                str argument_name = gimme(tokenizer, 'i');
                if(argument_name.id < 0 || gimme(tokenizer, ':').id < 0) return (Node) {
                    .prototype = &Error,
                    .error = {
                        .perror = &UnexpectedToken,
                        .label = argument_name,
                    },
                };
                Node argument_type = Type(tokenizer);

                push(&signature, argument_type);
                push(&argument_names, argument_name);

                if(tokenizer->id == ',') next(tokenizer);
                else break;
            }
            if(gimme(tokenizer, ')').id < 0) return (Node) {
                .prototype = &Error,
                .error = {
                    .perror = &UnexpectedToken,
                    .label = next(tokenizer),
                },
            };

            if(tokenizer->id == 'i' && streq(*tokenizer, constr("__external"))) {
                next(tokenizer);
                if(gimme(tokenizer, ';').id < 0) return (Node) {
                    .prototype = &Error,
                    .error = {
                        .perror = &UnexpectedToken,
                        .label = next(tokenizer),
                    },
                };

                push(&var_names, identifier);
                push(&vars, (Node) {
                    .prototype = &Variable,
                    .type = Box((Node) {
                        .prototype = &FunctionType,
                        .defined_type = {
                            .declaration = Box((Node) {
                                .prototype = &FunctionDeclaration,
                                .function_declaration = {
                                    .signature = signature,
                                    .identifier = identifier,
                                },
                            }),
                        }
                    }),
                    .variable = {
                        .identifier = identifier,
                    }
                });

                return (Node) { .prototype = &Ignore };
            }

            if(tokenizer->id == 'r') {
                next(tokenizer);
                signature[0] = Type(tokenizer);
            }
            
            str give = gimme(tokenizer, '{');
            if(give.id < 0) return (Node) {
                .prototype = &Error,
                .error = {
                    .perror = &UnexpectedToken,
                    .label = give,
                },
            };

            Node declaration = {
                .prototype = &FunctionDeclaration,
                .function_declaration = {
                    .signature = signature,
                    .argument_names = argument_names,
                    .identifier = identifier,
                    .body = Box(Body(tokenizer, '}')),
                },
            };

            push(&var_names, identifier);
            push(&vars, (Node) {
                .prototype = &Variable,
                .type = Box((Node) {
                    .prototype = &FunctionType,
                    .defined_type = {
                        .declaration = Box(declaration),
                    }
                }),
                .variable = {
                    .identifier = identifier,
                }
            });
            return declaration;
        }

        if(streq(*tokenizer, constr("return"))) {
            next(tokenizer);

            Node value = Postfix(tokenizer, Literal(tokenizer), 100);
            if(value.prototype == &Error) return value;

            str semicolon = gimme(tokenizer, ';');
            if(semicolon.id < 0) return (Node) {
                .prototype = &Error,
                .error = {
                    .perror = &UnexpectedToken,
                    .label = semicolon,
                },
            };

            return (Node) {
                .prototype = &ReturnStatement,
                .return_statement = {
                    .value = Box(value),
                },
            };
        }

        if(streq(*tokenizer, constr("let"))) {
            next(tokenizer);
            
            str identifier = gimme(tokenizer, 'i');
            if(identifier.id < 0) return (Node) {
                .prototype = &Error,
                .error = {
                    .perror = &UnexpectedToken,
                    .label = identifier,
                },
            };

            Node type = (Node) { .prototype = &Auto };
            if(tokenizer->id == ':') {
                next(tokenizer);
                type = Type(tokenizer);
                if(type.prototype == &Error) return type;
            }

            switch(tokenizer->id) {
                case '=': {
                    next(tokenizer);

                    Node value = Postfix(tokenizer, Literal(tokenizer), 100);
                    str semicolon = gimme(tokenizer, ';');
                    if(semicolon.id < 0) return (Node) {
                        .prototype = &Error,
                        .error = {
                            .perror = &UnexpectedToken,
                            .label = semicolon,
                        },
                    };

                    type_match(&type, *value.type);
                    Node* type_box = Box(type);

                    push(&var_names, identifier);
                    push(&vars, (Node) {
                        .prototype = &Variable,
                        .type = type_box,
                        .variable = {
                            .identifier = identifier,
                        },
                    });

                    return (Node) {
                        .prototype = &VariableDeclaration,
                        .variable_declaration = {
                            .type = type_box,
                            .identifier = identifier,
                            .value = Box(value),
                        },
                    };
                }

                case ';': {
                    next(tokenizer);
                    Node* type_box = Box(type);

                    push(&var_names, identifier);
                    push(&vars, (Node) {
                        .prototype = &Variable,
                        .type = type_box,
                        .variable = {
                            .identifier = identifier,
                        },
                    });

                    return (Node) {
                        .prototype = &VariableDeclaration,
                        .variable_declaration = {
                            .type = type_box,
                            .identifier = identifier,
                        },
                    };
                }

                default: return (Node) {
                    .prototype = &Error,
                    .error = {
                        .perror = &UnexpectedToken,
                        .label = next(tokenizer),
                    },
                };
            }
        }

        if(streq(*tokenizer, constr("struct"))) {
            next(tokenizer);

            str identifier = gimme(tokenizer, 'i');
            if(identifier.id < 0 || gimme(tokenizer, '{').id < 0) return (Node) {
                .prototype = &Error,
                .error = {
                    .perror = &UnexpectedToken,
                    .label = identifier,
                },
            };

            str* structure_keys = 0;
            Node* structure_types = 0;

            while(tokenizer->id > 0 && tokenizer->id != '}') {
                str key = gimme(tokenizer, 'i');
                if(key.id < 0) return (Node) {
                    .prototype = &Error,
                    .error = {
                        .perror = &UnexpectedToken,
                        .label = identifier,
                    },
                };

                Node type = { .prototype = &Auto };
                if(tokenizer->id == ':') {
                    next(tokenizer);
                    type = Type(tokenizer);
                }

                push(&structure_keys, key);
                push(&structure_types, type);

                if(tokenizer->id == ',') next(tokenizer);
                else break;
            }

            if(tokenizer->id != '}') return (Node) {
                .prototype = &Error,
                .error = {
                    .perror = &UnexpectedToken,
                    .label = next(tokenizer),
                },
            };
            next(tokenizer);

            Node declaration = {
                .prototype = &StructureDeclaration,
                .structure_declaration = {
                    .identifier = identifier,
                    .keys = structure_keys,
                    .types = structure_types,
                },
            };

            push(&type_names, identifier);
            push(&types, (Node) {
                .prototype = &StructureType,
                .defined_type = {
                    .declaration = Box(declaration),
                }
            });

            return declaration;
        }
    }

    Node expression = Postfix(tokenizer, Literal(tokenizer), 100);
    if(expression.prototype == &Error) return expression;

    str semicolon = gimme(tokenizer, ';');
    if(semicolon.id < 0) return (Node) {
        .prototype = &Error,
        .error = {
            .perror = &UnexpectedToken,
            .label = semicolon,
        },
    };

    return (Node) {
        .prototype = &StatementWrapper,
        .child = Box(expression),
    };
}

Node Body(str* tokenizer, int terminator) {
    Node body = {
        .prototype = &BodyPrototype,
        .children = 0,
    };

    while(tokenizer->id > 0 && tokenizer->id != terminator) {
        push(&body.children, Statement(tokenizer));
    }
    if(terminator && tokenizer->id == terminator) next(tokenizer);

    return body;
}