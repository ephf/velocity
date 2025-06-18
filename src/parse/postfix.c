#include "prefix.c"

#include "node/expression/binary_operation.c"
#include "node/data/function.c"
#include "node/data/structure.c"

void FieldAccess(Node node, FILE* file) {
    Node* parent = node.field_access.parent;
    parent->prototype(*parent, file);
    fprintf(file, ".%.*s", node.field_access.field.len, node.field_access.field.data);
}

Node* Expression(Tokenizer*, int);

Node* Postfix(Tokenizer* tokenizer, Node* left, int precedence) {
    int continue_loop = 1;
    continue_loop: while(continue_loop) {
        if(left->type->prototype == &FunctionType && tokenizer->current.type == '<') {
            left = parse_function_call(tokenizer, left);
            continue;
        }

        switch(tokenizer->current.type) {
            case '(': left = parse_function_call(tokenizer, left); break;
            
            case 'r': /* -> */ {
                next_token(tokenizer);
                left = dereference(left);
                while(try_token(tokenizer, '*', 0)) left = dereference(left);
            }
            case '.': {
                try_token(tokenizer, '.', 0);
                if(left->attributes & aStructure) {
                    push(&errors, cannot_find(left->range, "structure"));
                    goto continue_loop;
                }

                TokenResult field = expect_token(tokenizer, TokenIdentifier);
                if(field.is_error) goto continue_loop;

                Node* namespace = left->type->base_type.declaration->structure_declaration.namespace;
                Node** type = get(left->type->base_type.declaration->structure_declaration.body, field.token.str);

                if(!type) {
                    Node** function = get(namespace->namespace.body->body.context.variables, field.token.str);
                    if(!function || (*function)->type->prototype != &FunctionType) {
                        push(&errors, cannot_find(field.token, "structure function"));
                        goto continue_loop;
                    }

                    Vec(Node*) bounded_function_arguments_clone = 0;
                    for(int i = 0; i < len((*function)->variable.bounded_function_arguments); i++)
                        push(&bounded_function_arguments_clone, (*function)->variable.bounded_function_arguments[i]);

                    Vec(Node*) signature = (*function)->type->base_type.declaration->function_declaration.signature;
                    Node** self_arg_type = len(signature) < len(bounded_function_arguments_clone) + 1
                        ? (Node*[]) { &void_type } : &signature[len(bounded_function_arguments_clone) + 1];
                    
                    if((*self_arg_type)->attributes & aPointer) left = reference(left);
                    push(&bounded_function_arguments_clone, left);
                    
                    if(!type_match(self_arg_type, left->type))
                        push(&errors, type_mismatch(field.token, left->type, *self_arg_type));

                    left = Box((Node) { &Variable, 0, stretch_token(left->range, field.token),
                        .type = (*function)->type,
                        .variable = { (*function)->variable.identifier, bounded_function_arguments_clone }
                    });
                    goto continue_loop;
                }

                left = Box((Node) { &FieldAccess, 0, stretch_token(left->range, field.token),
                    .type = *type,
                    .field_access = { left, field.token.str },
                });
                goto continue_loop;
            }
        }

        left = parse_binary_operation(tokenizer, left, precedence, &continue_loop);
    }

    return left;
}

Node* Expression(Tokenizer* tokenizer, int precedence) {
    return Postfix(tokenizer, Prefix(tokenizer), precedence);
}