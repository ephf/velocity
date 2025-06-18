// <velocity test v0.1.0>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
// -- hoist section

int echo__int(int );
int main();

// -- parsed generics section

int echo__int(int x) {
    return x;
}

// -- std library section


// -- actual file section

int main() {
    int x = echo__int__int(echo__int(5));
}
