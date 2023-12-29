#ifndef PUREMVC_DICTIONARY_H
#define PUREMVC_DICTIONARY_H

#include <stdbool.h>

typedef struct Map Map;

struct Map {
    const char *key;
    const void *value;
    Map *next;
};

typedef struct Dictionary Dictionary;

struct Dictionary {
    Map *map;
    bool (*containsKey)(Dictionary *self, const char *key);
    const void *(*get)(Dictionary *self, const char *key);
    const void *(*put)(Dictionary *self, const char *key, const void *value);
    const void *(*replace)(Dictionary *self, const char *key, const void *value);
    const void *(*removeItem)(Dictionary *self, const char *key);
    void (*clear)(Dictionary *self, void (*callback)(void *value));
};

Dictionary *puremvc_dictionary_alloc();

Dictionary *puremvc_dictionary_init(Dictionary *self);

Dictionary *puremvc_dictionary_new();

void puremvc_dictionary_free(Dictionary **self);

#endif //PUREMVC_DICTIONARY_H
