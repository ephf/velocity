#ifndef TOK
#define TOK

#include "lib.c"

#define numc(x) ((x) >= '0' && (x) <= '9')
#define alpnumc(x) ((x) >= 'a' && (x) <= 'z' || (x) >= 'A' && (x) <= 'Z' || (x) == '_' || numc(x))

char lookup[128] = { 0 };
char lookup_chars[] = "!%&*+-/<=>?^|~:";

__attribute__ ((constructor))
void init_lookup() {
    for(char i = 0; i < sizeof(lookup_chars) - 1; i++) {
        lookup[lookup_chars[i]] = i;
    }
}

str next(str* p) {
    str n = *p;
    if(!n.id) return (str) { -1, 13, "<end of file>" };
    p->data += n.len;
    p->len = 0;
    while(*p->data && *p->data <= ' ') p->data++;
    if(numc(*p->data)) {
        do p->len++; while(numc(p->data[p->len]));
        p->id = 'n';
        return n;
    }
    if(alpnumc(*p->data)) {
        do p->len++; while(alpnumc(p->data[p->len]));
        p->id = 'i';
        return n;
    }
    if(*p->data == '"') {
        while(p->data[++p->len] && p->data[p->len] != '"') {
            if(p->data[p->len] == '\\') p->len++;
        }
        p->len++;
        p->id = '"';
        return n;
    }
    if(p->data[0] == '-' && p->data[1] == '>') {
        p->len = 2;
        p->id = 'r';
        return n;
    }
    if(lookup[p->data[0]] && lookup[p->data[1]]) {
        p->len = 2;
        p->id = 'A' + lookup[*p->data];
        return n;
    }
    if(lookup[p->data[0]] && p->data[1] == '=') {
        p->len = 2;
        p->id = 'A' + 14 + lookup[*p->data];
        return n;
    }
    *p = (str) { *p->data, 1, p->data };
    return n;
}

str gimme(str* p, int id) {
    if(p->id != id) return (str) { -1, p->len, p->data };
    return next(p);
}

#endif