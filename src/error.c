#ifndef ERROR_C
#define ERROR_C

#include "token.c"

struct Node;

typedef struct Error {
    const char* label;
    const char* highlight;

    Token fault_point;
    void (*perror)(struct Error);
    Vec(struct Error) attachments;

    union {
        struct { int expected_type; } as_expected_other;
        struct { char* possible_types; } as_unexpected_token;
        struct {
            struct Node* a;
            struct Node* b;
        } as_type_mismatch;
        struct { const char* what; } as_cannot_find;
        struct { const char* keyword; } as_expected_keyword;
    };
} Error;

Vec(Error) errors = 0;

void print_error_collection(Vec(Error) errors) {
    for(int i = 0; i < len(errors); i++) {
        char underline[strlen(errors[i].fault_point.trace.line_ptr) + 1];

        for(int j = 0; j < sizeof(underline) - 1; j++)
            underline[j] =
                j >= (errors[i].fault_point.str.data - errors[i].fault_point.trace.line_ptr)
                && j < (errors[i].fault_point.str.data - errors[i].fault_point.trace.line_ptr) + errors[i].fault_point.str.len
                ? '~' : ' ';
        underline[sizeof(underline) - 1] = 0;

        fprintf(stderr, "(main) %d:%d: %s%s:\33[0m ",
            errors[i].fault_point.trace.line, errors[i].fault_point.trace.character,
            errors[i].highlight, errors[i].label);
        errors[i].perror(errors[i]);
        fprintf(stderr, "\n %4d | %s\n      : %s%s\33[0m\n ",
            errors[i].fault_point.trace.line,
            errors[i].fault_point.trace.line_ptr,
            errors[i].highlight, underline);

        if(errors[i].attachments) print_error_collection(errors[i].attachments);
        fprintf(stderr, "\n");
    }
}

bool print_errors() {
    if(!errors) return false;

    fprintf(stderr, "compiled with %lu \33[31merror(s)\33[0m:\n\n", len(errors));
    print_error_collection(errors);
    return true;
}

Error create_basic_error(Error template) {
    template.label = "error";
    template.highlight = "\33[31m";
    return template;
}

Error create_see_error(Error template) {
    template.label = "see";
    template.highlight = "\33[33m";
    return template;
}

void UnexpectedTokenError(Error error) {
    str fault = error.fault_point.str;
    char* possible_types = error.as_unexpected_token.possible_types;

    fprintf(stderr, "Unexpected token \33[36m%.*s\33[0m", fault.len, fault.data);

    if(possible_types) {
        fprintf(stderr, ", expected one of: ");

        for(; *possible_types; possible_types++) {
            str type = stringify_token_type((int) *possible_types);
            fprintf(stderr, "\33[36m%.*s\33[0m", type.len, type.data);
            if(possible_types[1]) fprintf(stderr, ", ");
        }
    }
}

Error unexpected_token(Token token, char* possible_types) {
    return create_basic_error((Error) {
        .fault_point = token,
        .perror = &UnexpectedTokenError,
        .as_unexpected_token = { possible_types },
    });
}

void CannotFindError(Error error) {
    fprintf(stderr, "Cannot find %s \33[36m%.*s\33[0m",
        error.as_cannot_find.what,
        error.fault_point.str.len, error.fault_point.str.data);
}

Error cannot_find(Token identifier, const char* what) {
    return create_basic_error((Error) {
        .fault_point = identifier,
        .perror = &CannotFindError,
        .as_cannot_find = { what },
    });
}

void ExpectedKeywordError(Error error) {
    str fault = error.fault_point.str;
    const char* keyword = error.as_expected_keyword.keyword;

    fprintf(stderr, "expected keyword \33[36m%s\33[0m, but got \33[36m%.*s\33[0m",
        keyword, fault.len, fault.data);
}

Error expected_keyword(Token token, const char* keyword) {
    return create_basic_error((Error) {
        .fault_point = token,
        .perror = &ExpectedKeywordError,
        .as_expected_keyword = { keyword },
    });
}

void SeeDeclaration(Error error) {
    fprintf(stderr, "see declaration of \33[36m%.*s\33[0m",
        error.fault_point.str.len, error.fault_point.str.data);
}

#endif