#include "postfix.c"

#include "node/data/variable.c"
#include "node/data/function.c"
#include "node/data/structure.c"

int initial_indent = 0;
void BodyPrototype(Node node, FILE* file) {
    if(initial_indent++) push(&indent, "    ");
    for(int i = 0; i < len(node.body.children); i++) {
        fprintf(file, "%.*s", (int) len(indent) * 4, (char*)(void*) indent);
        Node child = node.body.children[i];
        child.prototype(child, file);
    }
    if(--initial_indent) pop(indent);
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

Node Body(str*, int, int);

Node Statement(str* tokenizer) {
    if(tokenizer->id == 'i') {
        if(streq(*tokenizer, constr("fn")))
            return parse_function_declaration(tokenizer);

        if(streq(*tokenizer, constr("return"))) {
            next(tokenizer);

            Node value = Postfix(tokenizer, Literal(tokenizer), 100);
            if(gimme(tokenizer, ';').id < 0)
                return unexpected_token(*tokenizer);

            return (Node) {
                .prototype = &ReturnStatement,
                .return_statement = {
                    .value = Box(value),
                },
            };
        }

        if(streq(*tokenizer, constr("let")))
            return parse_variable_declaration(tokenizer);

        if(streq(*tokenizer, constr("struct"))) 
            return parse_structure_declaration(tokenizer);
    }

    Node expression = Expression(tokenizer, 100);
    if(expression.prototype == &Error) return expression;
    if(gimme(tokenizer, ';').id < 0) return unexpected_token(*tokenizer);

    return (Node) {
        .prototype = &StatementWrapper,
        .child = Box(expression),
    };
}

Node Body(str* tokenizer, int terminator, int library) {
    Node body_node = {
        .prototype = &BodyPrototype,
        .body = { 0 },
    };

    push(&stack, (Context) { 0 });

    while(tokenizer->id > 0 && tokenizer->id != terminator) {
        push(&body_node.body.children, Statement(tokenizer));
    }
    if(terminator && tokenizer->id == terminator) next(tokenizer);

    body_node.body.context = stack[len(stack) - 1];
    if(!library) pop(stack);

    return body_node;
}