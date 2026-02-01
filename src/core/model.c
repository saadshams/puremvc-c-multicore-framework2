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
#include "puremvc/mutex.h"

// The Multiton Model instanceMap.
static struct Model instanceMap[INSTANCE_MAP_SIZE];

// mutex for instanceMap
static MutexOnce token = MUTEX_ONCE_INIT;
static Mutex mutex;

static void initializeModel(struct Model *self) {
    (void)self;
}

static void registerProxy(struct Model *self, struct Proxy proxy) {
    mutex_lock(&self->proxyMapMutex);
    size_t i = 0;
    for (; i < PROXY_MAP_SIZE && self->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->proxyMap[i].key, proxy.getName(&proxy)) == 0) {
            self->proxyMap[i].proxy.onRemove(&self->proxyMap[i].proxy);
            memset(&self->proxyMap[i].proxy, 0, sizeof(struct Proxy));
            break;
        }
    }

    if (i >= PROXY_MAP_SIZE) return mutex_unlock(&self->proxyMapMutex), (void)0; // proxyMap is full

    proxy.notifier.initializeNotifier(&proxy.notifier, self->multitonKey);

    snprintf(self->proxyMap[i].key, KEY_SIZE, "%s", proxy.name);
    self->proxyMap[i].proxy = proxy;
    self->proxyMap[i].proxy.onRegister(&self->proxyMap[i].proxy);
    mutex_unlock(&self->proxyMapMutex);
}

static struct Proxy *retrieveProxy(struct Model *self, const char *proxyName) {
    mutex_lock_shared(&self->proxyMapMutex);
    for (size_t i = 0; i < PROXY_MAP_SIZE && self->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->proxyMap[i].key, proxyName) == 0) {
            return mutex_unlock(&self->proxyMapMutex), &self->proxyMap[i].proxy;
        }
    }
    return mutex_unlock(&self->proxyMapMutex), NULL;
}

static bool hasProxy(struct Model *self, const char *proxyName) {
    mutex_lock_shared(&self->proxyMapMutex);
    bool exists = false;
    for (size_t i = 0; i < PROXY_MAP_SIZE && self->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->proxyMap[i].key, proxyName) == 0) {
            exists = true;
            break;
        }
    }
    return mutex_unlock(&self->proxyMapMutex), exists;
}

static struct Proxy removeProxy(struct Model *self, const char *proxyName) {
    mutex_lock(&self->proxyMapMutex);
    struct Proxy proxy = {0};
    size_t index = 0;
    for (size_t i = 0; i < PROXY_MAP_SIZE && self->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->proxyMap[i].key, proxyName) == 0) {
            proxy = self->proxyMap[i].proxy;
            proxy.onRemove(&proxy);
            memset(&self->proxyMap[index], 0, sizeof(struct ProxyMap));
        } else {
            if (index != i) { // shift left
                memmove(&self->proxyMap[index], &self->proxyMap[i], sizeof(struct ProxyMap));
                memset(&self->proxyMap[i], 0, sizeof(struct ProxyMap));
            }
            index++;
        }
    }

    return mutex_unlock(&self->proxyMapMutex), proxy;
}

struct Model puremvc_model(const char *key) {
    struct Model model = {
        .initializeModel = initializeModel,
        .registerProxy = registerProxy,
        .retrieveProxy = retrieveProxy,
        .hasProxy = hasProxy,
        .removeProxy = removeProxy
    };

    snprintf(model.multitonKey, KEY_SIZE, "%s", key);
    return model;
}

static void dispatchOnce() {
    mutex_init(&mutex);
}

struct Model *puremvc_model_getInstance(const char *key, struct Model(*factory)(const char *key)) {
    if (key == NULL || factory == NULL) return NULL;
    mutex_once(&token, dispatchOnce);
    mutex_lock(&mutex);

    size_t i = 0;
    for (; instanceMap[i].multitonKey[0] != '\0'; i++) {
        if (strncmp(instanceMap[i].multitonKey, key, KEY_SIZE) == 0) {
            return mutex_unlock(&mutex), &instanceMap[i];
        }
    }

    if (i >= INSTANCE_MAP_SIZE) return mutex_unlock(&mutex), NULL;

    instanceMap[i] = factory(key);

    return mutex_unlock(&mutex), &instanceMap[i];
}

void puremvc_model_removeModel(const char *key) {
    if (key == NULL) return;
    mutex_once(&token, dispatchOnce);
    mutex_lock(&mutex);

    size_t index = 0;
    for (size_t i = 0; i < INSTANCE_MAP_SIZE && instanceMap[i].multitonKey[0] != '\0'; i++) {
        if (strcmp(instanceMap[i].multitonKey, key) == 0) {
            memset(&instanceMap[i], 0, sizeof(struct Model));
        } else {
            if (index != i) {
                memmove(&instanceMap[index], &instanceMap[i], sizeof(struct Model));
                memset(&instanceMap[i], 0, sizeof(struct Model));
            }
            index++;
        }
    }
    mutex_unlock(&mutex);
}
