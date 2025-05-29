#include "parse/statement.c"

int main() {
    str std_tokenizer = { -1, 0, readfile("lib/velocity/std.vl") };
    next(&std_tokenizer);
    Node std = Body(&std_tokenizer, 0, 1);
    std.prototype(std, fopen("/dev/null", "w"));

    str tokenizer = { -1, 0, readfile("test/main.vl") };
    next(&tokenizer);

    Node program = Body(&tokenizer, 0, 0);
    FILE* out = fopen("test/main.c", "w+");
    fprintf(out, "#include \"../lib/c/std.c\"\n// <velocity test v0>\n\n");

    program.prototype(program, out);
}