#ifndef PUREMVC_MODEL_H
#define PUREMVC_MODEL_H

#include "proxy.h"
#include "dictionary.h"

struct Model {
    const char *multitonKey;
    Dictionary *proxyMap;

    void (*initializeModel)(struct Model *self);
    struct Proxy *(*registerProxy)(struct Model *self, struct Proxy *proxy);
    struct Proxy *(*retrieveProxy)(struct Model *self, const char *proxyName);
    bool (*hasProxy)(struct Model *self, const char *proxyName);
    struct Proxy *(*removeProxy)(struct Model *self, const char *proxyName);
};

struct Model *puremvc_model_init(struct Model *self);

struct Model *puremvc_model_alloc(const char *key);

struct Model *puremvc_model_new(const char *key);

void puremvc_model_free(struct Model **self);

struct Model *puremvc_model_getInstance(const char *key, struct Model *(*factory)(const char *));

void puremvc_model_removeModel(const char *key);

#endif //PUREMVC_MODEL_H
