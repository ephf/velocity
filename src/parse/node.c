#ifndef NODE_C
#define NODE_C

#include "../tok.c"
#include "../lib.c"

typedef struct Node {
    void (*prototype)(struct Node, FILE*);
    union {
        struct {
            Vec(struct Node) children;
            struct Context {
                Map(struct Node) variables;
                Map(struct Node) types;
                struct Node* current_function;
            } context;
        } body;
        
        struct Node* child;

        struct {
            int type_meta;
            union {
                struct {
                    union {
                        const char* c_type;
                        struct Node* declaration;
                    };
                    struct Node* child;
                } base_type;

                struct {
                    struct Node* base;
                    Vec(struct Node*) variadic_arguments;
                    Vec(char) modifiers;
                } modified_type;
            };
        };

        struct {
            int meta;
            Vec(struct Node*) signature;
            Vec(str) argument_names;
            str identifier;
            struct Node* body;
        } function_declaration;

        struct {
            struct Node* type;
            str identifier;
            struct Node* value;
        } variable_declaration;

        struct {
            str identifier;
            Map(struct Node*) body;
        } structure_declaration;

        struct {
            struct Node* value;
        } return_statement;

        struct {
            struct Node* type;
            union {
                struct {
                    str identifier;
                } variable;

                struct {
                    int type;
                    union {
                        long number;
                        str string;
                    };
                } constant;

                struct {
                    struct Node* left;
                    str operator;
                    struct Node* right;
                } binary_operation;

                struct {
                    void (*perror)(struct Node);
                    union {
                        struct {
                            const char* cannot_find;
                            str label;
                        };
                        struct {
                            struct Node* a;
                            struct Node* b;
                        } mismatch;
                    };
                } error;

                struct {
                    struct Node* function;
                    Vec(struct Node) arguments;
                } function_call;

                struct {
                    Map(struct Node) body;
                } structure;
            };
        };
    };
} Node;

enum {
    cNumber = 0,
    cString,

    tIsNumeric = 1 << 0,

    fExternal = 1 << 0,

    mPointer =  1 << 0,
    mArray =    1 << 1,
};

char (*indent)[4] = 0;

typedef struct Context Context;

Vec(Context) stack = 0;

#endif