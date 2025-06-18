#include "postfix.c"

#include "node/data/variable.c"
#include "node/data/function.c"
#include "node/data/structure.c"
#include "node/statement/namespace.c"
#include "node/data/type.c"
#include "node/symbol/identifier.c"

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

Node* Body(Tokenizer*, int, int, Node*);

Node* Statement(Tokenizer* tokenizer) {
    if(tokenizer->current.type == TokenIdentifier) {
        if(streq(tokenizer->current.str, constr("fn")))
            return parse_function_declaration(tokenizer);

        if(streq(tokenizer->current.str, constr("return"))) {
            TokenResult range_start = next_token(tokenizer);

            Node* value = Postfix(tokenizer, Literal(tokenizer), 100);
            expect_token(tokenizer, ';');

            return Box((Node) { &ReturnStatement, 0, stretch_token(range_start.token, value->range),
                .return_statement = { value }
            });
        }

        if(streq(tokenizer->current.str, constr("let")) || streq(tokenizer->current.str, constr("const")))
            return parse_variable_declaration(tokenizer);

        if(streq(tokenizer->current.str, constr("struct"))) 
            return parse_structure_declaration(tokenizer);

        if(streq(tokenizer->current.str, constr("namespace"))) {
            TokenResult range_start = next_token(tokenizer);
            ResolvedIdentifier ri = resolve_identifier(tokenizer, 1);

            Node* namespace_node = Box((Node) { &Namespace, 0, stretch_token(range_start.token, ri.base),
                .type = &void_type,
                .namespace = {
                    .identifier = ri.constructed,
                    .body = Box((Node) { &BodyPrototype })
                },
            });

            Node* body_node = namespace_node->namespace.body;
            body_node->body.context = (Context) { .namespace = namespace_node };

            put(&ri.scope->variables, ri.base.str, namespace_node);

            expect_token(tokenizer, '{');
            Body(tokenizer, '}', 0, body_node);

            return body_node;
        }

        if(streq(tokenizer->current.str, constr("type")))
            return parse_type_declaration(tokenizer);
    }

    Node* expression = Expression(tokenizer, 100);
    TokenResult range_end = expect_token(tokenizer, ';');

    return Box((Node) { &StatementWrapper, 0,
        stretch_token(expression->range, range_end.token), .child = expression, });
}

Node* Body(Tokenizer* tokenizer, int terminator, int library, Node* body_node) {
    if(!body_node) body_node = Box((Node) { .prototype = &BodyPrototype });
    push(&stack, &body_node->body.context);

    while(tokenizer->current.type > 0 && tokenizer->current.type != terminator) {
        push(&body_node->body.children, Statement(tokenizer));
    }
    if(terminator) try_token(tokenizer, terminator, 0);

    if(!library) pop(stack);
    return body_node;
}