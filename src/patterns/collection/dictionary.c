#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dictionary.h"

// mutex for Map
static pthread_rwlock_t mutex = PTHREAD_RWLOCK_INITIALIZER;

static bool containsKey(Dictionary *self, const char *key) {
    pthread_rwlock_rdlock(&mutex);
    for (Map *cursor = self->map; cursor; cursor = cursor->next) {
        if (strcmp(cursor->key, key) == 0) {
            pthread_rwlock_unlock(&mutex);
            return true;
        }
    }
    pthread_rwlock_unlock(&mutex);
    return false;
}

static const void *get(Dictionary *self, const char *key) {
    pthread_rwlock_rdlock(&mutex);
    for (Map *cursor = self->map; cursor; cursor = cursor->next) {
        if (strcmp(cursor->key, key) == 0) {
            pthread_rwlock_unlock(&mutex);
            return cursor->value;
        }
    }
    pthread_rwlock_unlock(&mutex);
    return NULL;
}

static const void *put(Dictionary *self, const char *key, const void *value) {
    if (containsKey(self, key)) return NULL;

    pthread_rwlock_wrlock(&mutex);
    Map *node = malloc(sizeof(Map));
    if (node == NULL) goto exception;

    node->key = strdup(key);
    node->value = value;
    node->next = NULL;

    Map **cursor;
    for(cursor = &self->map; *cursor; cursor = &(*cursor)->next);
    *cursor = node;

    pthread_rwlock_unlock(&mutex);
    return node->value;

    exception:
    pthread_rwlock_unlock(&mutex);
    fprintf(stderr, "Map allocation failed.\n");
    return NULL;
}

static const void *replace(Dictionary *self, const char *key, const void *value) {
    pthread_rwlock_wrlock(&mutex);

    for (Map *cursor = self->map; cursor; cursor = cursor->next) {
        if (strcmp(cursor->key, key) == 0) {
            const void *oldValue = cursor->value;
            cursor->value = value;

            pthread_rwlock_unlock(&mutex);
            return oldValue;
        }
    }

    pthread_rwlock_unlock(&mutex);
    return NULL;
}

static const void *removeItem(Dictionary *self, const char *key) {
    pthread_rwlock_wrlock(&mutex);

    for (Map **cursor = &self->map; *cursor; cursor = &(*cursor)->next) {
        Map *node = *cursor;
        if (strcmp(node->key, key) == 0) {
            *cursor = node->next;
            const void *value = node->value;

            free((void *) node->key);
            free(node);

            pthread_rwlock_unlock(&mutex);
            return value;
        }
    }

    pthread_rwlock_unlock(&mutex);
    return NULL;
}

static void clear(Dictionary *dictionary, void (*callback)(void *value)) {
    Map *current = dictionary->map;
    while (current != NULL) {
        Map *node = current;
        current = current->next;
        free((void *) node->key);
        if (callback) callback((void *) node->value);
        free(node);
    }
    dictionary->map = NULL;
}

Dictionary *puremvc_dictionary_init(Dictionary *self) {
    self->map = NULL;
    self->containsKey = containsKey;
    self->get = get;
    self->put = put;
    self->replace = replace;
    self->removeItem = removeItem;
    self->clear = clear;
    return self;
}

Dictionary *puremvc_dictionary_alloc() {
    Dictionary *self = malloc(sizeof(Dictionary));
    if (self == NULL) goto exception;
    memset(self, 0, sizeof(*self));
    return self;

    exception:
    fprintf(stderr, "Dictionary allocation failed.\n");
    return NULL;
}

Dictionary *puremvc_dictionary_new() {
    return puremvc_dictionary_init(puremvc_dictionary_alloc());
}

void puremvc_dictionary_free(Dictionary **self) {
    free(*self);
    *self = NULL;
}
