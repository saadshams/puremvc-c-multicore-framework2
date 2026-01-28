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
static struct Model instanceMap[MAX_INSTANCES];
static size_t count = 0;  // number of instances

// mutex for instanceMap
// static MutexOnce token = MUTEX_ONCE_NT;
// static Mutex mutex;

static void initializeModel(struct Model *self) {

}

static void registerProxy(struct Model *self, struct Proxy *proxy) {
    // mutex_lock(&this->proxyMapMutex);
    proxy->notifier.initializeNotifier(&proxy->notifier, self->multitonKey);

    for (size_t i =  0; i < self->proxyCount; i++) { // replace
        if (strcmp(self->proxyMap[i].getName(&self->proxyMap[i]), proxy->getName(proxy)) == 0) {
            self->proxyMap[i] = *proxy;
            proxy->onRegister(proxy);
            return;
        }
    }

    if (self->proxyCount < MAX_PROXIES) { // insert
        self->proxyMap[self->proxyCount++] = *proxy;
        proxy->onRegister(proxy);
    }
    // mutex_unlock(&this->proxyMapMutex);
}

static struct Proxy *retrieveProxy(struct Model *self, const char *proxyName) {
    // mutex_lock_shared(&this->proxyMapMutex);
    // struct Proxy *proxy = (struct Proxy *) this->proxyMap->get(this->proxyMap, proxyName);
    for (size_t i = 0; i < self->proxyCount; i++) {
        if (strcmp(self->proxyMap[i].getName(&self->proxyMap[i]), proxyName) == 0) {
            return &self->proxyMap[i];
        }
    }
    // mutex_unlock(&this->proxyMapMutex);
    // return proxy;
    return NULL;
}

static bool hasProxy(const struct Model *self, const char *proxyName) {
    // mutex_lock_shared(&this->proxyMapMutex);
    // const bool exists = this->proxyMap->containsKey(this->proxyMap, proxyName);
    bool exists = false;
    for (size_t i = 0; i < self->proxyCount; i++) {
        if (strcmp(self->proxyMap[i].getName(&self->proxyMap[i]), proxyName) == 0) {
            exists = true;
            break;
        }

    }
    // mutex_unlock(&this->proxyMapMutex);
    return exists;
}

static struct Proxy removeProxy(struct Model *self, const char *proxyName) {
    // mutex_lock(&this->proxyMapMutex);
    // struct Proxy *proxy = this->proxyMap->removeItem(this->proxyMap, proxyName);
    for (size_t i = 0; i < self->proxyCount; i++) { // remove
        if (strcmp(self->proxyMap[i].getName(&self->proxyMap[i]), proxyName) == 0) {
            struct Proxy proxy = self->proxyMap[i];

            for (size_t j = i + 1; j < self->proxyCount; j++) { // shift left
                self->proxyMap[j-1] = self->proxyMap[j];
            }

            self->proxyCount--;
            proxy.onRemove(&proxy);
            return proxy;
        }
    }
    // mutex_unlock(&this->proxyMapMutex);
    struct Proxy empty = {0}; // default return if not found
    return empty;
}

struct Model puremvc_model(const char *key) {
    struct Model model = {};

    snprintf(model.multitonKey, MAX_NAME_LEN, "%s", key);

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
    // mutex_once(&token, dispatchOnce);
    // mutex_lock(&mutex);

    for (size_t i = 0; i < count; i++) { // get
        if (strncmp(instanceMap[i].multitonKey, key, MAX_NAME_LEN) == 0) {
            return &instanceMap[i];
        }
    }

    if (count >= MAX_INSTANCES) return NULL;

    instanceMap[count] = factory(key);

    // mutex_unlock(&mutex);
    return &instanceMap[count++];
}

void puremvc_model_removeModel(const char *key) {
    if (key == NULL) return;
    // mutex_once(&token, dispatchOnce);
    // mutex_lock(&mutex);
    for (size_t i = 0; i < count; i++) {
        if (strncmp(instanceMap[i].multitonKey, key, MAX_NAME_LEN) == 0) { // remove
            memset(&instanceMap[i], 0, sizeof(struct Model));

            for (size_t j = i + 1; j < count; j++) // shift left
                instanceMap[j-1] = instanceMap[j];

            count--;
            break;
        }
    }
    // mutex_unlock(&mutex);
}
