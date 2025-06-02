#include "literal.c"

Node* Prefix(str* tokenizer) {
    return Literal(tokenizer);
}