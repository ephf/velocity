#include "postfix.c"

#include "node/data/variable.c"
#include "node/data/function.c"
#include "node/data/structure.c"
#include "node/statement/namespace.c"
#include "node/data/type.c"

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

Node* Body(str*, int, int, Node*);

Node* Statement(str* tokenizer) {
    if(tokenizer->id == 'i') {
        if(streq(*tokenizer, constr("fn")))
            return parse_function_declaration(tokenizer);

        if(streq(*tokenizer, constr("return"))) {
            next(tokenizer);

            Node* value = Postfix(tokenizer, Literal(tokenizer), 100);
            if(gimme(tokenizer, ';').id < 0)
                return unexpected_token(*tokenizer);

            return Box((Node) {
                .prototype = &ReturnStatement,
                .return_statement = {
                    .value = value,
                },
            });
        }

        if(streq(*tokenizer, constr("let")))
            return parse_variable_declaration(tokenizer);

        if(streq(*tokenizer, constr("struct"))) 
            return parse_structure_declaration(tokenizer);

        if(streq(*tokenizer, constr("namespace"))) {
            next(tokenizer);

            str identifier = gimme(tokenizer, 'i');
            if(identifier.id < 0 || gimme(tokenizer, '{').id < 0)
                return unexpected_token(*tokenizer);

            Node* namespace_node = Box((Node) {
                .prototype = &Namespace,
                .namespace = {
                    .identifier = identifier,
                    .body = Box((Node) {
                        .prototype = &BodyPrototype,
                        .body = {
                            .children = 0,
                        }
                    })
                },
            });

            Node* body_node = namespace_node->namespace.body;
            body_node->body.context = (Context) {
                .namespace = namespace_node,
            };

            put(&stack[len(stack) - 1].variables, identifier, namespace_node);

            Body(tokenizer, '}', 0, body_node);
            return Box((Node) { .prototype = &Ignore });
        }

        if(streq(*tokenizer, constr("type")))
            return parse_type_declaration(tokenizer);
    }

    Node* expression = Expression(tokenizer, 100);
    if(expression->prototype == &Error) return expression;
    if(gimme(tokenizer, ';').id < 0) return unexpected_token(*tokenizer);

    return Box((Node) {
        .prototype = &StatementWrapper,
        .child = expression,
    });
}

Node* Body(str* tokenizer, int terminator, int library, Node* body_node) {
    if(!body_node) body_node = Box((Node) { .prototype = &BodyPrototype });
    push(&stack, body_node->body.context);

    while(tokenizer->id > 0 && tokenizer->id != terminator) {
        push(&body_node->body.children, Statement(tokenizer));
    }
    if(terminator && tokenizer->id == terminator) next(tokenizer);

    body_node->body.context = stack[len(stack) - 1];
    if(!library) pop(stack);

    return body_node;
}