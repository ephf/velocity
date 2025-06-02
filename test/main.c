#include "../lib/c/std.c"
// <velocity test v0>

int main() {
    typeof(int*) box = malloc(4);
    realloc(box, 0);
    return 0;
}
