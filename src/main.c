#include "parse/statement.c"

int main() {
    Tokenizer std_tokenizer = tokenizer(readfile("lib/std.vl"));
    Tokenizer main_tokenizer = tokenizer(readfile("test/main.vl"));

    // Node* std_section = Body(&std_tokenizer, 0, 1, 0);
    Node* main_section = Body(&main_tokenizer, 0, 0, 0);

    if(errors) {
        print_errors();
        return 1;
    }

    FILE* out = fopen("test/main.c", "w+");
    fprintf(out, "// <velocity test v0.1.0>\n#include <stdio.h>\n#include <stdlib.h>\n#include <stdbool.h>\n// -- hoist section\n\n");

    hoist_section->prototype(*hoist_section, out);
    hoisted = 0;

    fprintf(out, "\n// -- parsed generics section\n\n");
    generic_section->prototype(*generic_section, out);

    fprintf(out, "\n// -- std library section\n\n");
    // std_section->prototype(*std_section, out);

    fprintf(out, "\n// -- actual file section\n\n");
    main_section->prototype(*main_section, out);
}