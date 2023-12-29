#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "array.h"

// mutex for List
static pthread_rwlock_t mutex = PTHREAD_RWLOCK_INITIALIZER;

// Inserts the specified element at the beginning of this list (unshift).
static const void *unshift(Array *self, const void *item) {
    pthread_rwlock_wrlock(&mutex);
    List *node = malloc(sizeof(List));
    if (node == NULL) goto exception;

    node->item = item;
    node->next = self->list;
    self->list = node;

    pthread_rwlock_unlock(&mutex);
    return item;

    exception:
    pthread_rwlock_unlock(&mutex);
    fprintf(stderr, "List allocation failed.\n");
    return NULL;
}

// Appends the specified element to the end of this list (push).
static const void *push(Array *self, const void *item) {
    pthread_rwlock_wrlock(&mutex);
    List *node = malloc(sizeof(List));
    if (node == NULL) goto exception;

    node->item = item;
    node->next = NULL;

    List **cursor;
    for (cursor = &self->list; *cursor; cursor = &(*cursor)->next);
    *cursor = node;

    pthread_rwlock_unlock(&mutex);
    return node->item;

    exception:
    pthread_rwlock_unlock(&mutex);
    fprintf(stderr, "List allocation failed.\n");
    return NULL;
}

// Removes and returns the first element from this list (shift).
static const void *shift(Array *self) {
    pthread_rwlock_wrlock(&mutex);

    List *node = self->list;
    if (node) {
        self->list = node->next;
        pthread_rwlock_unlock(&mutex);

        const void *item = node->item;
        free(node);
        pthread_rwlock_unlock(&mutex);
        return item;
    }

    pthread_rwlock_unlock(&mutex);
    return NULL;
}

// pop
static const void *pop(Array *self) {
    pthread_rwlock_wrlock(&mutex);

    List **cursor;
    for (cursor = &self->list; *cursor && (*cursor)->next; cursor = &(*cursor)->next);

    if (*cursor) {
        List *node = *cursor;
        *cursor = NULL;
        pthread_rwlock_unlock(&mutex);

        const void *item = node->item;
        free(node);
        pthread_rwlock_unlock(&mutex);
        return item;
    }

    pthread_rwlock_unlock(&mutex);
    return NULL;
}

static const void *removeItem(Array *self, const void *item) {
    pthread_rwlock_wrlock(&mutex);

    for (List **cursor = &self->list; *cursor; cursor = &(*cursor)->next) {
        if ((*cursor)->item == item) {
            List *node = *cursor;
            *cursor = (*cursor)->next;
            pthread_rwlock_unlock(&mutex);

            const void *data = node->item;
            free(node);
            pthread_rwlock_unlock(&mutex);
            return data;
        }
    }

    pthread_rwlock_unlock(&mutex);
    return NULL;
}

static bool containsValue(Array *self, const void *item) {
    pthread_rwlock_rdlock(&mutex);

    for (List *cursor = self->list; cursor; cursor = cursor->next) {
        if (cursor->item == item) {
            pthread_rwlock_unlock(&mutex);
            return true;
        }
    }

    pthread_rwlock_unlock(&mutex);
    return false;
}

static Array *clone(Array *self) {
    pthread_rwlock_rdlock(&mutex);

    List *copy = NULL;
    for (List *cursor = self->list, **copyPtr = &copy; cursor; cursor = cursor->next) {
        List *node = malloc(sizeof(List));
        if (!node) goto exception;

        node->item = cursor->item;
        node->next = NULL;
        *copyPtr = node;
        copyPtr = &node->next;
    }

    pthread_rwlock_unlock(&mutex);
    Array *arr = puremvc_array_new();
    arr->list = copy;
    return arr;

    exception:
    pthread_rwlock_unlock(&mutex);
    fprintf(stderr, "Node allocation failed.\n");
    return NULL;
}

static int size(Array *self) {
    pthread_rwlock_rdlock(&mutex);

    int count = 0;
    for (List *cursor = self->list; cursor; cursor = cursor->next, count++);

    pthread_rwlock_unlock(&mutex);
    return count;
}

static void clear(Array *self, void (*callback)(void *item)) {
    pthread_rwlock_wrlock(&mutex);

    for (List **cursor = &self->list; *cursor;) {
        List *node = *cursor;
        *cursor = (*cursor)->next;
        if (callback) callback((void *) node->item);
        free(node);
    }

    pthread_rwlock_unlock(&mutex);
}

Array *puremvc_array_init(Array *self) {
    self->list = NULL;
    self->unshift = unshift;
    self->push = push;
    self->shift = shift;
    self->pop = pop;
    self->removeItem = removeItem;
    self->containsValue = containsValue;
    self->clone = clone;
    self->size = size;
    self->clear = clear;
    return self;
}

Array *puremvc_array_alloc() {
    Array *self = malloc(sizeof(Array));
    if (self == NULL) goto exception;
    memset(self, 0, sizeof(*self));
    return self;

    exception:
    fprintf(stderr, "Array allocation failed.\n");
    return NULL;
}

Array *puremvc_array_new() {
    return puremvc_array_init(puremvc_array_alloc());
}

void puremvc_array_free(Array **self) {
    free(*self);
    *self = NULL;
}
