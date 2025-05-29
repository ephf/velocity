#include "../lib/c/std.c"
// <velocity test v0>

typeof(int) main() {
    print((struct str) { .size = 11, .data = "Hello World", });
    return 0;
}
