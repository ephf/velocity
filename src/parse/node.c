#ifndef NODE_C
#define NODE_C

#include "../tokenizer.c"
#include "../lib.c"

struct GenericWrapper;

typedef struct Node {
    void (*prototype)(struct Node, FILE*);
    int attributes;
    Token range;
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
            Vec(int) modifiers;
        } modified_type;

        struct {
            Map(int) declaration_map;
            Vec(struct GenericWrapper)* declaration_sub_generics;
            Vec(struct Node*) declaration_generics;

            struct {
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
                struct Node* namespace;
            } structure_declaration;
        };

        struct {
            struct Node* value;
        } return_statement;

        struct {
            struct Node* type;
            union {
                struct GenericWrapper {
                    Vec(struct Node*) generics;
                    Vec(struct Node*)* generics_override;
                    str identifier;
                    Vec(str*) identifier_overrides;
                    struct Node* child;
                } generic_wrapper;

                struct {
                    str identifier;
                    Vec(struct Node*) bounded_function_arguments;
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

                Error error;

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
                    struct Node* parent_type;
                } namespace;

                struct {
                    struct Node* parent;
                    str field;
                } field_access;

                struct Node* sizeof_;

                struct Node* dereference;
                struct Node* reference;
            };
        };
    };
} Node;

enum {
    aNumeric    = 1 << 0,
    aConst      = 1 << 1,
    aHidden     = 1 << 2,
    aGeneric    = 1 << 3,
    aExternal   = 1 << 4,
    aStructure  = 1 << 5,
    aPointer    = 1 << 6,

    mPointer    = 1 << 0,
    mArray      = 1 << 1,

    cNumber     = 0,
    cString     = 1,
};

typedef struct Context Context;

Vec(Context*) stack = 0;

Node* hoist_section;
int hoisted = 1;

Node* generic_section;

void BodyPrototype(Node, FILE*);
__attribute__ ((constructor))
void _init_function_declarations() {
    hoist_section = Box((Node) { &BodyPrototype });
    generic_section = Box((Node) { &BodyPrototype });
}

Vec(Vec(struct GenericWrapper)*) generics_stack = 0;

#endif