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

// mutex for instanceMap
// static MutexOnce token = MUTEX_ONCE_NT;
// static Mutex mutex;

static void initializeModel(struct Model *self) {

}

static void registerProxy(struct Model *self, struct Proxy proxy) {
    // mutex_lock(&self->proxyMapMutex);
    size_t i = 0;
    for (; i < PROXY_MAP_SIZE && self->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->proxyMap[i].key, proxy.getName(&proxy)) == 0) {
            self->proxyMap[i].proxy.onRemove(&self->proxyMap[i].proxy);
            memset(&self->proxyMap[i].proxy, 0, sizeof(struct Proxy));
            break;
        }
    }

    if (i >= PROXY_MAP_SIZE) return; // proxyMap is full // mutex_unlock(&self->proxyMapMutex);

    proxy.notifier.initializeNotifier(&proxy.notifier, self->multitonKey);

    snprintf(self->proxyMap[i].key, KEY_SIZE, "%s", proxy.name);
    self->proxyMap[i].proxy = proxy;
    self->proxyMap[i].proxy.onRegister(&self->proxyMap[i].proxy);
    // mutex_unlock(&self->proxyMapMutex);
}

static struct Proxy *retrieveProxy(struct Model *self, const char *proxyName) {
    // mutex_lock_shared(&this->proxyMapMutex);
    for (size_t i = 0; i < PROXY_MAP_SIZE && self->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->proxyMap[i].key, proxyName) == 0) {
            return &self->proxyMap[i].proxy; // mutex_unlock(&self->proxyMapMutex);
        }
    }
    // mutex_unlock(&this->proxyMapMutex);
    return NULL;
}

static bool hasProxy(const struct Model *self, const char *proxyName) {
    // mutex_lock_shared(&this->proxyMapMutex);
    bool exists = false;
    for (size_t i = 0; i < PROXY_MAP_SIZE && self->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->proxyMap[i].key, proxyName) == 0) {
            exists = true;
            break;
        }
    }
    // mutex_unlock(&this->proxyMapMutex);
    return exists;
}

static struct Proxy removeProxy(struct Model *self, const char *proxyName) {
    // mutex_lock(&this->proxyMapMutex);
    struct ProxyMap *proxyMap = NULL;
    struct Proxy proxy = {0};

    size_t index = 0; // One-pass removal (Filter pattern)
    for (size_t i = 0; i < PROXY_MAP_SIZE && self->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->proxyMap[i].key, proxyName) == 0) {
            proxyMap = &self->proxyMap[i];
        } else {
            if (index != i) { // shift left
                memmove(&self->proxyMap[index], &self->proxyMap[i], sizeof(struct Proxy));
                memset(&self->proxyMap[i], 0, sizeof(struct ProxyMap));
            }
            index++;
        }
    }
    // mutex_unlock(&this->proxyMapMutex);

    if (proxyMap != NULL) {
        proxyMap->proxy.onRemove(&proxyMap->proxy);
        proxy = proxyMap->proxy;
        memset(proxyMap, 0, sizeof(struct ProxyMap));
    }

    return proxy;
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

    size_t i = 0;
    for (; instanceMap[i].multitonKey[0] != '\0'; i++) {
        if (strncmp(instanceMap[i].multitonKey, key, KEY_SIZE) == 0) {
            return &instanceMap[i];
        }
    }

    if (i >= INSTANCE_MAP_SIZE) return NULL;

    instanceMap[i] = factory(key);

    // mutex_unlock(&mutex);
    return &instanceMap[i];
}

void puremvc_model_removeModel(const char *key) {
    if (key == NULL) return;
    // mutex_once(&token, dispatchOnce);
    // mutex_lock(&mutex);

    for (size_t i = 0; i < INSTANCE_MAP_SIZE; i++) {
        if (strcmp(instanceMap[i].multitonKey, key) == 0) {
            memset(&instanceMap[i], 0, sizeof(struct Model));

            for (size_t j = i; j < INSTANCE_MAP_SIZE; j++) // shift left
                // instanceMap[j-1] = instanceMap[j];
                memmove(&instanceMap[j], &instanceMap[j + 1], sizeof(struct Model));
            break;
        }
    }
    // mutex_unlock(&mutex);
}
