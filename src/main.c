#include "parse/statement.c"

char extra_parse[] = "!%&*+-/<=>?^|~";

int main() {
    for(char i = 0; i < sizeof(extra_parse) - 1; i++) {
        lookup[extra_parse[i]] = i;
    }

    error_type = Box((Node) {
        .prototype = &Error,
        .error = {
            .perror = &AutoError,
        },
    });

    push(&type_names, constr("int"));
    push(&types, (Node) {
        .prototype = &CType,
        .defined_type = {
            .meta = tIsNumeric,
            .c_type = "int",
        },
    });
    push(&type_names, constr("usize"));
    push(&types, (Node) {
        .prototype = &CType,
        .defined_type = {
            .meta = tIsNumeric,
            .c_type = "size_t",
        }
    });
    push(&type_names, constr("char"));
    push(&types, (Node) {
        .prototype = &CType,
        .defined_type = {
            .meta = tIsNumeric,
            .c_type = "char",
        }
    });

    str std_tokenizer = { -1, 0, readfile("lib/velocity/std.vl") };
    next(&std_tokenizer);
    Node std = Body(&std_tokenizer, 0);
    std.prototype(std, fopen("/dev/null", "w"));

    str tokenizer = { -1, 0, readfile("test/main.vl") };
    next(&tokenizer);

    Node program = Body(&tokenizer, 0);
    FILE* out = fopen("test/main.c", "w+");
    fprintf(out, "#include \"../lib/c/std.c\"\n// <velocity test v0>\n\n");

    program.prototype(program, out);
}