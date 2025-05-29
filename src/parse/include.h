#ifndef INCLUDE_H
#define INCLUDE_H

#include "node.c"

int type_match(Node**, Node*);
Node* Type(str*);

Node Expression(str*, int);
Node Postfix(str*, Node, int);

Node Body(str*, int, int);

#endif