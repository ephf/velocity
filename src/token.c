#ifndef TOKEN_C
#define TOKEN_C

#include "lib.c"

char lookup_tokens[256] = { 0 };
const char lookup_token_types[] = "!%&*+-/<=>?^|~:";

__attribute__ ((constructor))
void init_lookup_tokens() {
    for(char i = 0; i < sizeof(lookup_token_types) - 1; i++)
        lookup_tokens[lookup_token_types[i]] = i;
}

typedef struct {
    int line;
    int character;
    char* line_ptr;
} Trace;

typedef struct {
    int type;
    str str;
    Trace trace;
} Token;

enum {
    TokenTypeStart = 256,
    TokenIdentifier = TokenTypeStart,
    TokenNumber,
    TokenString,
    TokenRightArrow,

    TokenDoubleLookupStart,

    TokenDoubleAmp          = TokenDoubleLookupStart + 2,
    TokenDoubleColon        = TokenDoubleLookupStart + 14,

    TokenEqualsLookupStart  = TokenDoubleLookupStart + sizeof(lookup_token_types) - 1,
};

str token_strings[] = {
    [TokenIdentifier - TokenTypeStart] = constr("identifier"),
    [TokenNumber - TokenTypeStart] = constr("number"),
    [TokenString - TokenTypeStart] = constr("string"),
    [TokenRightArrow - TokenTypeStart] = constr("->"),

    [TokenDoubleAmp - TokenTypeStart] = constr("&&"),
    [TokenDoubleColon - TokenTypeStart] = constr("::"),
};

#define stringify_token_type(type) \
    ((type) < TokenTypeStart ? (str) { 1, (char[]) { (type) } } : token_strings[(type) - TokenTypeStart])

Token stretch_token(Token start, Token end) {
    return (Token) {
        .str = { end.str.data + end.str.len - start.str.data, start.str.data },
        .trace = start.trace,
    };
}

#endif