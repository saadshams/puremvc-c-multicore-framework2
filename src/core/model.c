/**
* @file model.c
* @internal
* @brief Model mplementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/

#include "model.h"
#include "puremvc/i_proxy.h"

#include <stdio.h>
#include <string.h>

// instanceMap
static struct ModelMap **instanceMap = NULL;

// mutex for modelMap
static Mutex modelMapMutex;
static MutexOnce modelMutexOnce = MUTEX_ONCE_INIT;

static void initializeModel(struct IModel *self, struct ProxyMap **proxyMap) {
    struct Model *this = (struct Model *) self;
    this->proxyMap = proxyMap;
}

static bool registerProxy(struct IModel *self, struct IProxy *(*factory)(void *buffer, const char *name, void *data), const char *name, void *data) {
    struct Model *this = (struct Model *) self;

    if (this->proxyMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::registerProxy] FATAL: Missing ProxyMap field in ModelMap; skipping registration.\033[0m\n");
        return false;
    }

    mutex_lock(&this->proxyMapMutex);

    size_t i = 0;
    for (; this->proxyMap[i] != NULL && this->proxyMap[i]->key != NULL; i++) { // find existing
        if (this->proxyMap[i]->key == name || strcmp(this->proxyMap[i]->key, name) == 0) { // match
            this->proxyMap[i]->proxy->onRemove(this->proxyMap[i]->proxy);
            fprintf(stderr, "\033[0;33m[PureMVC::Model::registerMediator] Warning: Proxy '%s' exists; overridden registration\033[0m.\n", name);

            factory(this->proxyMap[i]->proxy, name, data); // re-registration
            mutex_unlock(&this->proxyMapMutex);
            return true;
        }
    }

    if (this->proxyMap[i] == NULL) { // overflow (ProxyMap)
        fprintf(stderr, "\033[0;31m[PureMVC::Model::registerProxy] Error: ProxyMap storage overflow for proxy '%s'; increase slots - skipping registration.\033[0m\n", name);
        mutex_unlock(&this->proxyMapMutex);
        return false;
    }

    // todo check if proxy exists (error if .proxy wasn't alloca)
    struct IProxy *proxy = factory(this->proxyMap[i]->proxy, name, data); // registration
    this->proxyMap[i]->key = proxy->getName(proxy);

    proxy->getNotifier(proxy)->initializeNotifier(proxy->getNotifier(proxy), this->multitonKey);

    mutex_unlock(&this->proxyMapMutex);

    proxy->onRegister(proxy);
    return true;
}

static struct IProxy *retrieveProxy(const struct IModel *self, const char *proxyName) {
    struct Model *this = (struct Model *) self;
    mutex_lock_shared(&this->proxyMapMutex);
    struct IProxy *proxy = NULL;
    for (size_t i = 0; this->proxyMap != NULL && this->proxyMap[i] != NULL && this->proxyMap[i]->key != NULL; i++) {
        if (this->proxyMap[i]->key == proxyName || strcmp(this->proxyMap[i]->key, proxyName) == 0) {
            proxy = this->proxyMap[i]->proxy;
            mutex_unlock(&this->proxyMapMutex);
            return proxy;
        }
    }

    mutex_unlock(&this->proxyMapMutex);
    return proxy;
}

static bool hasProxy(const struct IModel *self, const char *proxyName) {
    struct Model *this = (struct Model *) self;
    mutex_lock_shared(&this->proxyMapMutex);
    bool exists = false;
    for (size_t i = 0; this->proxyMap != NULL && this->proxyMap[i] != NULL && this->proxyMap[i]->key != NULL; i++) {
        if (this->proxyMap[i]->key == proxyName || strcmp(this->proxyMap[i]->key, proxyName) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&this->proxyMapMutex);
    return exists;
}

static bool removeProxy(struct IModel *self, const char *proxyName, struct IProxy **out) {
    struct Model *this = (struct Model *) self;
    bool removed = false;

    mutex_lock(&this->proxyMapMutex);

    size_t index = 0, i = 0;
    for (; this->proxyMap != NULL && this->proxyMap[i] != NULL && this->proxyMap[i]->key != NULL; i++) {
        if (this->proxyMap[i]->key == proxyName || strcmp(this->proxyMap[i]->key, proxyName) == 0) { // match
            if (out != NULL)
                *out = this->proxyMap[i]->proxy;

            this->proxyMap[i]->proxy->onRemove(this->proxyMap[i]->proxy);

            this->proxyMap[i]->key = NULL; // remove key only, proxy is borrowed
            removed = true;
        } else {
            if (index != i) { // shift left
                *this->proxyMap[index] = *this->proxyMap[i]; // shift left first
                this->proxyMap[i]->key = NULL; // remove key only
            }
            index++;
        }
    }

    mutex_unlock(&this->proxyMapMutex);
    return removed;
}

size_t puremvc_model_size() {
    return (sizeof(struct Model) + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1);
}

struct IModel *puremvc_model_init(void *buffer, const char *key) {
    struct Model *this = (struct Model *) buffer;

    memset(this, 0, sizeof(struct Model));

    this->base.initializeModel = initializeModel;
    this->base.registerProxy = registerProxy;
    this->base.retrieveProxy = retrieveProxy;
    this->base.hasProxy = hasProxy;
    this->base.removeProxy = removeProxy;

    this->multitonKey = key;
    mutex_init(&this->proxyMapMutex);

    return (struct IModel *) this;
}

static void dispatchOnce(void) {
    mutex_init(&modelMapMutex);
}

struct IModel *puremvc_model_getInstance(struct ModelMap **modelMap, const char *key) {
    if (modelMap == NULL && instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] FATAL: Missing ModelMap storage; skipping registration.\033[0m\n");
        return NULL;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] FATAL: Key is NULL; skipping registration.\033[0m\n");
        return NULL;

        // fputs("[PureMVC::Model::getInstance] Error: Key '", stderr);
        // fputs(key, stderr);
        // fputs("' too long — skipping registration.\n", stderr);

        // fputs("\033[0;31m", stderr);      // 1. Terminal: "Okay, everything is RED now."
        // fputs("[PureMVC] ", stderr);      // 2. Terminal: Prints "[PureMVC]" in red.
        // fputs("FATAL ERROR", stderr);     // 3. Terminal: Prints "FATAL ERROR" in red.
        // fputs("\033[0m\n", stderr);       // 4. Terminal: "Back to normal colors," then Newline.
    }

    if (instanceMap == NULL)
        instanceMap = modelMap;

    mutex_once(&modelMutexOnce, dispatchOnce);
    mutex_lock(&modelMapMutex);

    size_t i = 0;
    for (; instanceMap[i] != NULL && instanceMap[i]->key != NULL; i++) { // find model
        if (instanceMap[i]->key == key || strcmp(instanceMap[i]->key, key) == 0) {
            mutex_unlock(&modelMapMutex);
            return instanceMap[i]->model;
        }
    }

    if (instanceMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] FATAL: ModelMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        // fputs("\033[0;31m[PureMVC::Model::getInstance] FATAL: Missing Model storage; skipping registration.\033[0m\n", stderr);
        mutex_unlock(&modelMapMutex);
        return NULL;
    }

    if (instanceMap[i]->model == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] FATAL: Missing Model storage; skipping registration.\033[0m\n");
        return NULL;
    }

    instanceMap[i]->key = key; // init
    puremvc_model_init(instanceMap[i]->model, key);

    mutex_unlock(&modelMapMutex);
    return instanceMap[i]->model;
}

bool puremvc_model_removeModel(const char *key, struct IModel **out) {
    if (instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::removeModel] FATAL: Missing ModelMap storage; skipping removal.\033[0m\n");
        return false;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::removeModel] FATAL: Key is NULL; skipping removal.\033[0m\n");
        return false;
    }

    mutex_once(&modelMutexOnce, dispatchOnce);
    mutex_lock(&modelMapMutex);

    size_t index = 0;
    for (size_t i = 0; instanceMap[i] != NULL && instanceMap[i]->key != NULL; i++) { // find model
        if (instanceMap[i]->key == key || strcmp(instanceMap[i]->key, key) == 0) {
            instanceMap[i]->key = NULL; // remove
            if (out != NULL)
                *out = instanceMap[i]->model;

            // ((struct Model *) s_modelMap[i]->model)->proxyMap[0]->key // todo remove proxies?
        } else {
            if (index != i) { // shift left
                *instanceMap[index] = *instanceMap[i];
                instanceMap[i]->key = NULL;
            }
            index++;
        }
    }

    if (index == 0) // all keys were removed; reset
        instanceMap = NULL;

    mutex_unlock(&modelMapMutex);

    return true;
}
