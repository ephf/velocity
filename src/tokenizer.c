#ifndef TOKENIZER_C
#define TOKENIZER_C

#include "lib.c"
#include "token.c"
#include "error.c"

typedef struct {
    Token current;
    Trace trace;
} Tokenizer;

bool match(char ch, char* matcher) {
    for(; *matcher; matcher++) {
        if(matcher[1] == '-') {
            if(ch >= matcher[0] && ch <= matcher[2]) return true;
            matcher += 2;
            continue;
        }

        if(ch == *matcher) return true;
    }
    return false;
}

Token create_token(char* data, Trace* trace) {
    while(*data && *data <= ' ') {
        trace->character++;
        if(*data++ == '\n') {
            data[-1] = 0;
            trace->line++;
            trace->character = 1;
            trace->line_ptr = data;
        }
    }
    
    int len = 0;

    if(match(*data, "a-zA-Z_")) {
        while(match(data[++len], "a-zA-Z0-9_"));
        trace->character += len;
        return (Token) { TokenIdentifier, { len, data }, *trace };
    }

    if(match(*data, "0-9")) {
        while(match(data[++len], "0-9_"));
        trace->character += len;
        return (Token) { TokenNumber, { len, data }, *trace };
    }

    if(*data == '"') {
        while(data[++len] && data[len] != '"') if(data[len] == '\\') len++;
        trace->character += ++len;
        return (Token) { TokenString, { len, data }, *trace };
    }

    if(*data == '-' && data[1] == '>') {
        trace->character += 2;
        return (Token) { TokenRightArrow, { 2, data }, *trace };
    }

    if(lookup_tokens[(unsigned) *data] && data[0] == data[1]) {
        trace->character += 2;
        return (Token) { TokenDoubleLookupStart + lookup_tokens[(unsigned) * data], { 2, data }, *trace };
    }

    if(lookup_tokens[(unsigned) *data] && data[1] == '=') {
        trace->character += 2;
        return (Token) { TokenEqualsLookupStart + lookup_tokens[(unsigned) * data], { 2, data }, *trace };
    }

    trace->character++;
    return (Token) { *data, { 1, data }, *trace };
}

Tokenizer tokenizer(char* data) {
    Trace trace = { 1, 1, data };
    Token current = create_token(data, &trace);
    return (Tokenizer) { current, trace };
}

typedef struct {
    bool is_error;
    Token token;
} TokenResult;

void ExpectedTokenError(Error error) {
    fprintf(stderr, "Expected a token, but got \33[36m<end of file>\33[0m");
}

TokenResult next_token(Tokenizer* tokenizer) {
    Token next = tokenizer->current;
    if(!next.type) {
        push(&errors, create_basic_error((Error) {
            .fault_point = next,
            .perror = &ExpectedTokenError,
        }));
        return (TokenResult) { true, next };
    }

    tokenizer->current = create_token(next.str.data + next.str.len, &tokenizer->trace);
    return (TokenResult) { false, next };
}

void ExpectedTokenTypeError(Error error) {
    str expected = stringify_token_type(error.as_expected_other.expected_type);
    str fault = error.fault_point.str;

    fprintf(stderr, "Expected a(n) \33[36m%.*s\33[0m, but got \33[36m%.*s\33[0m",
        expected.len, expected.data,
        fault.len, fault.data);
}

TokenResult expect_token(Tokenizer* tokenizer, int type) {
    TokenResult next = next_token(tokenizer);

    if(!next.is_error && next.token.type != type) {
        push(&errors, create_basic_error((Error) {
            .fault_point = next.token,
            .perror = &ExpectedTokenTypeError,
            .as_expected_other = { type },
        }));
    }

    return next;
}

bool try_token(Tokenizer* tokenizer, int type, Token* out) {
    if(tokenizer->current.type == type) {
        TokenResult next = next_token(tokenizer);
        if(out && !next.is_error) *out = next.token;
        return true;
    }
    return false;
}

#endif