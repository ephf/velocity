#ifndef INCLUDE_H
#define INCLUDE_H

#include "node.c"

void ModifiedType(Node, FILE*);
str stringify_type(Node*);
int type_match(Node**, Node*);
Error type_mismatch(Token, Node*, Node*);
Node* Type(Tokenizer*);

Node* Expression(Tokenizer*, int);
Node* Postfix(Tokenizer*, Node*, int);

Node* Body(Tokenizer*, int, int, Node*);

#endif