/**
* @file model.c
* @internal
* @brief Model mplementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include <stdio.h>
#include <string.h>

#include "puremvc/model.h"

// The Multiton Model instanceMap.
static struct Model instanceMap[INSTANCE_MAP_SIZE];
static size_t instanceMapCount = 0;  // number of instances

// mutex for instanceMap
// static MutexOnce token = MUTEX_ONCE_NT;
// static Mutex mutex;

static void initializeModel(struct Model *self) {

}

static void registerProxy(struct Model *self, struct Proxy proxy) {
    // mutex_lock(&this->proxyMapMutex);
    struct ProxyMap *slot = NULL;
    proxy.notifier.initializeNotifier(&proxy.notifier, self->multitonKey);

    for (size_t i = 0; i < self->proxyMapCount; i++) { // replace
        if (strcmp(self->proxyMap[i].key, proxy.getName(&proxy)) == 0) {
            slot = &self->proxyMap[i];
            slot->proxy.onRemove(&slot->proxy); // notify old proxy
            slot->proxy = proxy;
            slot->proxy.onRegister(&slot->proxy);
            return;
        }
    }

    if (self->proxyMapCount >= PROXIES_MAP_SIZE) return;

    slot = &self->proxyMap[self->proxyMapCount];
    snprintf(slot->key, KEY_SIZE, "%s", proxy.name);
    slot->proxy = proxy; // insert
    slot->proxy.onRegister(&slot->proxy);
    self->proxyMapCount++;
    // mutex_unlock(&this->proxyMapMutex);
}

static struct Proxy *retrieveProxy(struct Model *self, const char *proxyName) {
    // mutex_lock_shared(&this->proxyMapMutex);
    for (size_t i = 0; i < self->proxyMapCount; i++) {
        if (strcmp(self->proxyMap[i].key, proxyName) == 0) {
            return &self->proxyMap[i].proxy;
        }
    }
    // mutex_unlock(&this->proxyMapMutex);
    return NULL;
}

static bool hasProxy(const struct Model *self, const char *proxyName) {
    // mutex_lock_shared(&this->proxyMapMutex);
    bool exists = false;
    for (size_t i = 0; i < self->proxyMapCount; i++) {
        if (strcmp(self->proxyMap[i].key, proxyName) == 0) {
            exists = true;
            break;
        }
    }
    // mutex_unlock(&this->proxyMapMutex);
    return exists;
}

// suggestion is to return value
// model is passing all tests with pointer though
static struct Proxy removeProxy(struct Model *self, const char *proxyName) {
    // mutex_lock(&this->proxyMapMutex);
    struct ProxyMap *proxyMap = NULL;
    struct Proxy *proxy = NULL;
    struct Proxy value = {0};

    size_t index = 0; // One-pass removal (Filter pattern)
    for (size_t i = 0; i < self->proxyMapCount; i++) {
        if (strcmp(self->proxyMap[i].key, proxyName) == 0) {
            proxyMap = &self->proxyMap[i];
            proxy = &self->proxyMap[i].proxy;
        } else {
            if (index != i) { // shift left not getting iterated
                memmove(&self->proxyMap[index], &self->proxyMap[i], sizeof(struct Proxy));
                memset(&self->proxyMap[i], 0, sizeof(struct Proxy));
            }
            index++;
        }
    }

    // mutex_unlock(&this->proxyMapMutex);

    if (proxy != NULL) {
        proxy->onRemove(proxy);
        value = *proxy;

        memset(proxyMap, 0, sizeof(struct ProxyMap));
        self->proxyMapCount--;
    }

    return value;
}

struct Model puremvc_model(const char *key) {
    struct Model model = {0};

    snprintf(model.multitonKey, KEY_SIZE, "%s", key);

    model.initializeModel = initializeModel;
    model.registerProxy = registerProxy;
    model.retrieveProxy = retrieveProxy;
    model.hasProxy = hasProxy;
    model.removeProxy = removeProxy;

    return model;
}

static void dispatchOnce() {
    // mutex_init(&mutex);
}

struct Model *puremvc_model_getInstance(const char *key, struct Model(*factory)(const char *key)) {
    if (key == NULL || factory == NULL) return NULL;
    // mutex_once(&token, dispatchOnce);
    // mutex_lock(&mutex);

    for (size_t i = 0; i < instanceMapCount; i++) { // get
        if (strncmp(instanceMap[i].multitonKey, key, KEY_SIZE) == 0) {
            return &instanceMap[i];
        }
    }

    if (instanceMapCount >= INSTANCE_MAP_SIZE) return NULL;

    instanceMap[instanceMapCount] = factory(key);

    // mutex_unlock(&mutex);
    return &instanceMap[instanceMapCount++];
}

void puremvc_model_removeModel(const char *key) {
    if (key == NULL) return;
    // mutex_once(&token, dispatchOnce);
    // mutex_lock(&mutex);
    for (size_t i = 0; i < instanceMapCount; i++) {
        if (strcmp(instanceMap[i].multitonKey, key) == 0) {
            memset(&instanceMap[i], 0, sizeof(struct Model)); // remove

            for (size_t j = i + 1; j < instanceMapCount; j++) // shift left
                instanceMap[j-1] = instanceMap[j];

            instanceMapCount--;
            break;
        }
    }
    // mutex_unlock(&mutex);
}
