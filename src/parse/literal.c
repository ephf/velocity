#include "type.c"
#include "include.h"

#include "node/data/constant.c"
#include "node/data/variable.c"
#include "node/symbol/identifier.c"
#include "node/symbol/generics.c"

void Sizeof(Node node, FILE* file) {
    fprintf(file, "sizeof(");
    node.sizeof_->prototype(*node.sizeof_, file);
    fprintf(file, ")");
}

Node* Literal(Tokenizer* tokenizer) {
    TokenResult literal = next_token(tokenizer);

    switch(literal.token.type) {
        case TokenNumber: return parse_number_constant(literal.token);
        case TokenString: return parse_string_constant(literal.token);
        case TokenIdentifier: {
            if(streq(literal.token.str, constr("sizeof"))) {
                expect_token(tokenizer, '<');
                Node* type = Type(tokenizer);
                TokenResult range_end = expect_token(tokenizer, '>');

                return Box((Node) { &Sizeof, 0, stretch_token(literal.token, range_end.token),
                    .type = &numeric_type,
                    .sizeof_ = type,
                });
            }

            if(streq(literal.token.str, constr("external"))) {
                expect_token(tokenizer, '<');
                Node* type = Type(tokenizer);
                expect_token(tokenizer, '>');

                Token external_value;
                if(!try_token(tokenizer, TokenString, &external_value))
                    external_value = expect_token(tokenizer, TokenIdentifier).token;

                return Box((Node) { &Constant, 0, stretch_token(literal.token, external_value),
                    .type = type,
                    .constant = { cString, .string = external_value.type == TokenIdentifier
                        ? external_value.str
                        : (str) { .len = external_value.str.len - 2, .data = external_value.str.data + 1 } }
                });
            }

            ResolvedIdentifier ri = resolve_identifier_given(literal.token, tokenizer, 0);

            Node* variable;
            if(ri.scope) {
                Node** variable_search = get(ri.scope->variables, ri.base.str);
                variable = variable_search ? *variable_search : 0;
            } else variable = find_variable(ri.base.str);

            if(!variable) {
                push(&errors, cannot_find(ri.base, "variable"));
                return Box((Node) { &Ignore, 0, ri.base, .type = &void_type });
            }
            if(variable->prototype != &Namespace) return variable; 

            Node* type = 0;
            if(ri.scope) {
                Node** type_search = get(ri.scope->types, ri.base.str);
                type = type_search ? *type_search : 0;
            } else for(int i = len(stack); i--; ) {
                Node** type_search = get(stack[i]->types, ri.base.str);
                if(type_search) {
                    type = *type_search;
                    break;
                }
            }

            if(!type || type->prototype != &StructureType) {
                push(&errors, cannot_find(ri.base, "structure type"));
                return Box((Node) { &Ignore, 0, ri.base, .type = &void_type });
            }
            Node* structure = Box((Node) { &Structure, aStructure, stretch_token(literal.token, ri.base),
                .type = type });

            if(type->base_type.declaration->attributes & aGeneric) {
                reset_generics(type->base_type.declaration);
                if(try_token(tokenizer, '<', 0)) resolve_type_arguments(tokenizer,
                    &type->base_type.declaration->declaration_generics);
            }

            expect_token(tokenizer, '{');
            while(tokenizer->current.type != '}') {
                TokenResult key = expect_token(tokenizer, TokenIdentifier);
                if(key.is_error) continue;

                expect_token(tokenizer, ':');
                Node* value = Expression(tokenizer, 100);

                Node** key_type = get(type->base_type.declaration->structure_declaration.body, key.token.str);
                if(!key_type)
                    push(&errors, type_mismatch(stretch_token(key.token, value->range), value->type, &void_type));
                else if(!type_match(key_type, value->type))
                    push(&errors, type_mismatch(stretch_token(key.token, value->range), value->type, *key_type));
                else put(&structure->structure.body, key.token.str, value);

                if(!try_token(tokenizer, ',', 0) && tokenizer->current.type != '}') expect_token(tokenizer, ',');
            }
            expect_token(tokenizer, '}');

            if(type->base_type.declaration->attributes & aGeneric) {
                Vec(Node*) wrapped_children = generic_wrap_declaration(tokenizer,
                    (struct GenericWrapper) {
                        .generics = type->base_type.declaration->declaration_generics,
                        .generics_override = &type->base_type.declaration->declaration_generics,
                        .identifier = type->base_type.declaration->structure_declaration.identifier,
                        .identifier_overrides = vec(&type->base_type.declaration->structure_declaration.identifier),
                    }, type->base_type.declaration, vec(structure));
                return wrapped_children[0];
            }

            return structure;
        }
    }

    push(&errors, unexpected_token(literal.token, "i forgot to implement this shit correctly"));
    return Box((Node) { &Ignore, 0, literal.token, .type = &void_type });
}