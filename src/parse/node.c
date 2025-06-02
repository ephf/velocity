#ifndef NODE_C
#define NODE_C

#include "../tok.c"
#include "../lib.c"

typedef struct Node {
    void (*prototype)(struct Node, FILE*);
    union {
        struct {
            Vec(struct Node*) children;
            struct Context {
                Map(struct Node*) variables;
                Map(struct Node*) types;
                struct Node* current_function;
                struct Node* namespace;
            } context;
        } body;
        
        struct Node* child;

        struct {
            int type_meta;
            union {
                union {
                    str c_type;
                    struct Node* declaration;
                    struct {
                        Vec(struct Node*)* generics;
                        int index;
                    } generic;
                    struct {
                        struct Node* parent;
                        struct Node* child;
                    } generic_wrapper;
                } base_type;

                struct {
                    struct Node* base;
                    Vec(struct Node*) type_arguments;
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
            Vec(struct Node*) generics;
            Map(int) monomorphized_functions;
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
                    Vec(struct Node*) generics;
                    Vec(struct Node*)* generics_override;
                    str identifier;
                    Vec(str*) identifier_overrides;
                    struct Node* child;
                } generic_wrapper;

                struct {
                    int meta;
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
                    Vec(struct Node*) arguments;
                } function_call;

                struct {
                    Map(struct Node*) body;
                } structure;

                struct {
                    str identifier;
                    struct Node* body;
                } namespace;
            };
        };
    };
} Node;

enum {
    cNumber = 0,
    cString,

    tIsNumeric  = 1 << 0,
    tHidden     = 1 << 1,
    tAutoConst  = 1 << 2,

    fExternal   = 1 << 0,
    fGeneric    = 1 << 1,

    vHidden     = 1 << 0,

    mPointer    = 1 << 0,
    mArray      = 1 << 1,
};

typedef struct Context Context;

Vec(Context) stack = 0;

#endif