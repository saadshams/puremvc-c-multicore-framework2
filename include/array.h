#ifndef PUREMVC_ARRAY_H
#define PUREMVC_ARRAY_H

#include <stdbool.h>

typedef struct List List;

struct List {
    const void *item;
    struct List *next;
};

typedef struct Array Array;

struct Array {
    List *list;
    const void *(*unshift)(Array *self, const void *item);
    const void *(*push)(Array *self, const void *item);
    const void *(*shift)(Array *self);
    const void *(*pop)(Array *self);
    const void *(*removeItem)(Array *self, const void *item);
    bool (*containsValue)(Array *self, const void *item);
    Array *(*clone)(Array *self);
    int (*size)(Array *self);
    void (*clear)(Array *self, void (*callback)(void *item));
};

Array *puremvc_array_init(Array *self);

Array *puremvc_array_alloc();

Array *puremvc_array_new();

void puremvc_array_free(Array **self);

#endif //PUREMVC_ARRAY_H
