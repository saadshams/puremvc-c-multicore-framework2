/**
* @file model.c
* @internal
* @brief Model mplementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/

#include "model.h"

#include "puremvc/platform.h"
#include "puremvc/i_proxy.h"

#include <stdio.h>
#include <string.h>

// instanceMap
static struct ModelMap **model_instanceMap = NULL;

// mutex for modelMap
static Mutex modelMapMutex;
static MutexOnce modelMutexOnce = MUTEX_ONCE_INIT;

static void initializeModel(struct IModel *self, struct ProxyMap **proxyMap) {
    struct Model *this = (struct Model *) self;
    this->proxyMap = proxyMap;
}

static bool registerProxy(struct IModel *self, struct IProxy *(*factory)(void *buffer, const char *name, void *data), const char *name, void *data) {
    struct Model *this = (struct Model *) self;

    mutex_lock(&this->proxyMapMutex);

    if (this->proxyMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::registerProxy] FATAL: Missing ProxyMap field in ModelMap; skipping registration.\033[0m\n");
        return false;
    }

    size_t i = 0;
    for (; this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) { // find existing
        if (this->proxyMap[i]->key == name || strcmp(this->proxyMap[i]->key, name) == 0) {
            this->proxyMap[i]->proxy->onRemove(this->proxyMap[i]->proxy);
            fprintf(stderr, "\033[0;33m[PureMVC::Model::registerMediator] Warning: Proxy '%s' exists; overriding registration.\033[0m\n", name);

            struct IProxy *proxy = factory(this->proxyMap[i]->proxy, name, data); // replace

            struct INotifier *notifier = this->proxyMap[i]->proxy->getNotifier(this->proxyMap[i]->proxy);
            if (notifier == NULL) return false;
            notifier->initializeNotifier(notifier, this->multitonKey);

            if (proxy == NULL) return false;
            proxy->onRegister(proxy);

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
    struct IProxy *proxy = factory(this->proxyMap[i]->proxy, name, data); // init
    if (proxy == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Proxy::registerProxy] Error: Factory failed to initialize Proxy '%s' .\033[0m\n", name);
        return false;
    }

    struct INotifier *notifier = proxy->getNotifier(proxy);
    notifier->initializeNotifier(notifier, this->multitonKey);

    const char *key = proxy->getName(proxy);
    int len = snprintf(this->proxyMap[i]->key, KEY_SIZE, "%s", key); // registration
    if (len >= KEY_SIZE) { // todo reset proxy or init proxy after, you have the name, you have to reinit buffer too in this case
        fprintf(stderr, "\033[0;31m[PureMVC::Model::registerProxy] Error: ProxyMap key truncated: '%s' (max %d chars).\033[0m\n", key, KEY_SIZE);
        memset(this->proxyMap[i]->key, 0, KEY_SIZE);
        mutex_unlock(&this->proxyMapMutex);
        return false;
    }
    proxy->onRegister(proxy);

    mutex_unlock(&this->proxyMapMutex);
    return true;
}

static struct IProxy *retrieveProxy(const struct IModel *self, const char *proxyName) {
    struct Model *this = (struct Model *) self;
    mutex_lock_shared(&this->proxyMapMutex);
    struct IProxy *proxy = NULL;
    for (size_t i = 0; this->proxyMap != NULL && this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) {
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
    for (size_t i = 0; this->proxyMap != NULL && this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) {
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
    for (; this->proxyMap != NULL && this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) {
        if (this->proxyMap[i]->key == proxyName || strcmp(this->proxyMap[i]->key, proxyName) == 0) { // match
            if (out != NULL)
                *out = this->proxyMap[i]->proxy;

            struct IProxy *proxy = this->proxyMap[i]->proxy;
            proxy->onRemove(proxy);

            memset(&this->proxyMap[i]->key, 0, KEY_SIZE); // remove
            removed = true;
        } else {
            if (index != i) { // shift left (Gap-free array)
                *this->proxyMap[index] = *this->proxyMap[i]; // shift left first
                memset(&this->proxyMap[i]->key, 0, KEY_SIZE); // remove
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

    int len = snprintf(this->multitonKey, KEY_SIZE, "%s", key);
    if (len >= KEY_SIZE) {
        memset(this, 0, sizeof(struct Model));
        fprintf(stderr, "\033[0;31m[PureMVC::View::init] Error: Model multitonKey truncated: '%s' (max %d chars).\033[0m\n", key, KEY_SIZE);
        return NULL;
    }

    if (mutex_init(&this->proxyMapMutex) != 0) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::init] ERROR: ProxyMapMutex initialization failed for key '%s'.\033[0m\n", key);
        return NULL;
    }

    return (struct IModel *) this;
}

static void dispatchOnce(void) {
    mutex_init(&modelMapMutex);
}

struct IModel *puremvc_model_getInstance(struct ModelMap **modelMap, const char *key) {
    if (modelMap == NULL && model_instanceMap == NULL) {
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

    model_instanceMap = modelMap;

    mutex_once(&modelMutexOnce, dispatchOnce);
    mutex_lock(&modelMapMutex);

    size_t i = 0;
    for (; model_instanceMap != NULL && model_instanceMap[i] != NULL && model_instanceMap[i]->key[0] != '\0'; i++) { // find model
        if (model_instanceMap[i]->key == key || strcmp(model_instanceMap[i]->key, key) == 0) {
            mutex_unlock(&modelMapMutex);
            return model_instanceMap[i]->model;
        }
    }

    if (model_instanceMap == NULL || model_instanceMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] FATAL: ModelMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        // fputs("\033[0;31m[PureMVC::Model::getInstance] FATAL: Missing Model storage; skipping registration.\033[0m\n", stderr);
        mutex_unlock(&modelMapMutex);
        return NULL;
    }

    if (model_instanceMap[i]->model == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] FATAL: Missing Model storage; skipping registration.\033[0m\n");
        return NULL;
    }

    int len = snprintf(model_instanceMap[i]->key, KEY_SIZE, "%s", key); // registration
    if (len >= KEY_SIZE) { // todo reset proxy or init proxy after, you have the name
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] Error: ModelMap key truncated: '%s' (max %zu chars).\033[0m\n", key, sizeof(key));
        memset(model_instanceMap[i]->key, 0, KEY_SIZE);
        mutex_unlock(&modelMapMutex);
        return false;
    }

    puremvc_model_init(model_instanceMap[i]->model, key); // init

    mutex_unlock(&modelMapMutex);
    return model_instanceMap[i]->model;
}

bool puremvc_model_removeModel(const char *key, struct IModel **out) {
    bool removed = false;

    if (model_instanceMap == NULL) {
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
    for (size_t i = 0; model_instanceMap[i] != NULL && model_instanceMap[i]->key[0] != '\0'; i++) { // find model
        if (model_instanceMap[i]->key == key || strcmp(model_instanceMap[i]->key, key) == 0) {
            memset(model_instanceMap[i]->key, 0, KEY_SIZE); // remove
            if (out != NULL)
                *out = model_instanceMap[i]->model;

            removed = true;

            // ((struct Model *) s_modelMap[i]->model)->proxyMap[0]->key // todo remove proxies?
        } else {
            if (index != i) { // shift left
                *model_instanceMap[index] = *model_instanceMap[i];
                memset(model_instanceMap[i]->key, 0, KEY_SIZE); // remove
            }
            index++;
        }
    }

    mutex_unlock(&modelMapMutex);

    return removed;
}

void puremvc_model_reset() {
    mutex_lock(&modelMapMutex);
    model_instanceMap = NULL;
    mutex_unlock(&modelMapMutex);
}
