#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model.h"
#include "notifier.h"

// The Multiton Model instanceMap.
static Dictionary *instanceMap;

// mutex for instanceMap
static pthread_rwlock_t instanceMap_mutex = PTHREAD_RWLOCK_INITIALIZER;

// mutex for proxyMap
static pthread_rwlock_t proxyMap_mutex = PTHREAD_RWLOCK_INITIALIZER;

static void initializeModel(struct Model *self) {

}

static struct Proxy *registerProxy(struct Model *self, struct Proxy *proxy) {
    pthread_rwlock_wrlock(&proxyMap_mutex);
    proxy->notifier->initializeNotifier(proxy->notifier, self->multitonKey);
    if (self->proxyMap->containsKey(self->proxyMap, proxy->proxyName)) {
        struct Proxy *previous = (struct Proxy *) self->proxyMap->get(self->proxyMap, proxy->proxyName);
        self->proxyMap->replace(self->proxyMap, proxy->proxyName, proxy);
        pthread_rwlock_unlock(&proxyMap_mutex);
        return previous;
    } else {
        self->proxyMap->put(self->proxyMap, proxy->proxyName, proxy);
    }
    proxy->onRegister(proxy);
    pthread_rwlock_unlock(&proxyMap_mutex);
    return NULL;
}

static struct Proxy *retrieveProxy(struct Model *self, const char *proxyName) {
    pthread_rwlock_rdlock(&proxyMap_mutex);
    struct Proxy *proxy = (struct Proxy *) self->proxyMap->get(self->proxyMap, proxyName);
    pthread_rwlock_unlock(&proxyMap_mutex);
    return proxy;
}

static bool hasProxy(struct Model *self, const char *proxyName) {
    pthread_rwlock_rdlock(&proxyMap_mutex);
    bool exists = self->proxyMap->containsKey(self->proxyMap, proxyName);
    pthread_rwlock_unlock(&proxyMap_mutex);
    return exists;
}

static struct Proxy *removeProxy(struct Model *self, const char *proxyName) {
    pthread_rwlock_wrlock(&proxyMap_mutex);
    struct Proxy *proxy = (struct Proxy *) self->proxyMap->removeItem(self->proxyMap, proxyName);
    if (proxy)
        proxy->onRemove(proxy);
    pthread_rwlock_unlock(&proxyMap_mutex);
    return proxy;
}

struct Model *puremvc_model_init(struct Model *self) {
    self->initializeModel = initializeModel;
    self->registerProxy = registerProxy;
    self->retrieveProxy = retrieveProxy;
    self->hasProxy = hasProxy;
    self->removeProxy = removeProxy;
    return self;
}

struct Model *puremvc_model_alloc(const char *key) {
    assert(instanceMap->get(instanceMap, key) == NULL);

    struct Model *self = malloc(sizeof(struct Model));
    if (self == NULL) goto exception;
    memset(self, 0, sizeof(*self));

    puremvc_model_init(self);
    self->multitonKey = strdup(key);
    self->proxyMap = puremvc_dictionary_new();
    return self;

    exception:
    fprintf(stderr, "Model allocation failed.\n");
    return NULL;
}

struct Model *puremvc_model_new(const char *key) {
    return puremvc_model_init(puremvc_model_alloc(key));
}

void puremvc_model_free(struct Model **self) {
    free((void *) (*self)->multitonKey);
    (*self)->proxyMap->clear((*self)->proxyMap, free);
    puremvc_dictionary_free(&(*self)->proxyMap);

    free(*self);
    *self = NULL;
}

struct Model *puremvc_model_getInstance(const char *key, struct Model *(*factory)(const char *)) {
    pthread_rwlock_wrlock(&instanceMap_mutex);

    if (instanceMap == NULL) instanceMap = puremvc_dictionary_new();

    struct Model *instance = (struct Model *) instanceMap->get(instanceMap, key);
    if (instance == NULL) {
        instance = (struct Model *) instanceMap->put(instanceMap, key, factory(key));
        instance->initializeModel(instance);
    }
    pthread_rwlock_unlock(&instanceMap_mutex);
    return instance;
}

void puremvc_model_removeModel(const char *key) {
    pthread_rwlock_wrlock(&instanceMap_mutex);
    struct Model *model = (struct Model *) instanceMap->removeItem(instanceMap, key);
    if (model != NULL) puremvc_model_free(&model);
    pthread_rwlock_unlock(&instanceMap_mutex);
}
