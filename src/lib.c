#ifndef LIB
#define LIB

// I wouldn't recommend trying to read or understand this file
// Just know it contains useful library functions and data structures

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    int id, len;
    char* data;
} str;

#define constr(x) ((str) { 0, sizeof(x) - 1, (char*) x })
int streq(str a, str b) {
    return a.len == b.len && !strncmp(a.data, b.data, a.len);
}
#define strc(x...) ({ \
    str _s[] = { x }; \
    int _l = 0; \
    for(int i = 0; i < sizeof(_s) / sizeof(*_s); i++) _l += _s[i].len; \
    str _c = { 0, _l, malloc(_l) }; \
    for(int i = 0, j = 0; i < sizeof(_s) / sizeof(*_s); i++) { \
        memcpy(_c.data + j, _s[i].data, _s[i].len); \
        j += _s[i].len; \
    } \
    _c; })

#define Vec(T) T*
#define len(v) ((v) ? ((int*)(void*)(v))[-1] / sizeof(*(v)) : 0)
#define pop(v) (((int*)(void*)(v))[-1] -= sizeof(*(v)))
#define push(v, i...) _push((void*)(v), &(typeof(**(v))[]) { i }, sizeof(**(v)))
void _push(int** v, void* i, int n) {
    if(!*v) (*v = (int*) calloc(sizeof(int[2]) + n, 1) + 2)[-2] = n;
    if(((*v)[-1] += n) > (*v)[-2]) {
        while(((*v)[-2] *= 2) < (*v)[-1]);
        *v = (int*) realloc(*v - 2, sizeof(int[2]) + (*v)[-2]) + 2;
    }
    memcpy((void*) *v + (*v)[-1] - n, i, n);
}
#define vec(a, b...) ({ \
    typeof(a) _a[] = { a, b }; \
    Vec(typeof(a)) _v = 0; \
    for(int i = 0; i < sizeof(_a) / sizeof(*_a); i++) \
        push(&_v, _a[i]); \
    _v; })

#define MAPN 16
#define Map(T) Vec(T)*
unsigned _hash(str x) { // fnv-1
    unsigned h = 2166136261u;
    while(x.len--) h = (h ^ x.data[x.len]) * 16777619u;
    return h;
}
#define put(m, k, v...) _put((void*)(m), (void*) &(struct { str a; typeof(***(m)) b; }) { k, v }, sizeof(***(m)))
void _put(int*** m, str* kv, int n) {
    if(!*m) *m = calloc(sizeof(void*), MAPN);
    _push(*m + _hash(*kv) % MAPN, kv, n + sizeof(str));
}
#define get(m, k...) ((typeof(*(m))) _get((void*)(m), k, sizeof(**(m))))
void* _get(int** m, str k, int n) {
    if(m) {
        int* v = m[_hash(k) % MAPN];
        if(v) for(int i = 0; i < v[-1]; i += n + sizeof(str)) 
            if(streq(*(str*)((void*) v + i), k)) return (void*) v + i + sizeof(str);
    }
    return 0;
}
#define map(K, V, m...) ({ \
    typedef struct { K k; V v; } _struct; \
    _struct _m[] = m; \
    Map(V) _ma = 0; \
    for(int i = 0; i < sizeof(_m) / sizeof(_struct); i++) \
        put(&_ma, _m[i].k, _m[i].v); \
    _ma; })
#define entries(m, k, v) if(m) for(int i = 0, b = 1; i < MAPN; i++) \
    if((m)[i]) for(int j = 0; j < ((int**)(void*)(m))[i][-1]; (j += sizeof(str) + sizeof(**(m))), (b = 1)) \
        for(k = *(str*)((void*)((m)[i]) + j); b; ) \
        for(v = *(typeof(*(m)))((void*)((m)[i]) + j + sizeof(str)); b; b = 0)

#define Box(x...) ({ typeof(x) _x = (x); typeof(_x)* b = malloc(sizeof(_x)); memcpy(b, &_x, sizeof(_x)); b; })

char* readfile(char* n) {
    FILE* f = fopen(n, "r");
    fseek(f, 0, SEEK_END);
    char (*buf)[ftell(f)] = malloc(sizeof(*buf));
    rewind(f);
    (*buf)[fread(buf, 1, sizeof(*buf), f)] = 0;
    return (void*) buf;
}

#endif