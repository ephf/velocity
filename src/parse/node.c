#include "../tok.c"
#include "../lib.c"

typedef struct Node {
    void (*prototype)(struct Node, FILE*);
    union {
        struct Node* children;
        struct Node* child;

        struct {
            int meta;
            union {
                const char* c_type;
                struct Node* declaration;
            };
            struct Node* child;
        } defined_type;

        struct {
            int meta;
            struct Node* signature;
            str* argument_names;
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
            str* keys;
            struct Node* types;
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
                    struct Node* arguments;
                } function_call;

                struct {
                    str* keys;
                    struct Node* values;
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
};

str* type_names = 0;
Node* types = 0;

str* var_names = 0;
Node* vars;

char (*indent)[4] = 0;

void Ignore(Node node, FILE* file) {}