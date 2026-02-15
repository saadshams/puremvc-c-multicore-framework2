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

// instanceMap (global)
static struct ModelMap **instanceMap = NULL;

// mutex for modelMap (global)
static Mutex instanceMapMutex;
static MutexOnce mutexOnce = MUTEX_ONCE_INIT;

static const char *getMultitonKey(const struct IModel *self) {
    const struct Model *this = (const struct Model *) self;
    return this->multitonKey;
}

static void initializeModel(struct IModel *self, struct ProxyMap **proxyMap) {
    struct Model *this = (struct Model *) self;
    this->proxyMap = proxyMap;
}

static bool registerProxy(struct IModel *self, struct IProxy *(*factory)(void *buffer, const char *name, void *data), const char *name, void *data) {
    struct Model *this = (struct Model *) self;
    struct IProxy *proxy = NULL;
    bool registered = false;

    mutex_lock(&this->proxyMapMutex);

    if (this->proxyMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::registerProxy] ERROR: Missing ProxyMap field in ModelMap; skipping registration.\033[0m\n");
        goto finally;
    }

    size_t i = 0;
    for (; this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) { // find existing
        if (strcmp(this->proxyMap[i]->key, name) == 0) {
            this->proxyMap[i]->proxy->onRemove(this->proxyMap[i]->proxy);
            fprintf(stderr, "\033[0;33m[PureMVC::Model::registerMediator] Warning: Proxy '%s' exists; overriding registration.\033[0m\n", name);

            proxy = factory(this->proxyMap[i]->proxy, name, data); // replace
            proxy->getNotifier(proxy)->initializeNotifier(proxy->getNotifier(proxy), this->multitonKey);
            proxy->onRegister(proxy);
            registered = true;
            goto finally;
        }
    }

    if (this->proxyMap[i] == NULL) { // overflow (ProxyMap)
        fprintf(stderr, "\033[0;31m[PureMVC::Model::registerProxy] Error: ProxyMap storage overflow for proxy '%s'; increase slots - skipping registration.\033[0m\n", name);
        goto finally;
    }

    proxy = factory(this->proxyMap[i]->proxy, name, data); // init  todo check if proxy exists (error if .proxy wasn't alloca)
    proxy->getNotifier(proxy)->initializeNotifier(proxy->getNotifier(proxy), this->multitonKey);
    proxy->onRegister(proxy);

    const char *key = proxy->getName(proxy);
    int len = snprintf(this->proxyMap[i]->key, KEY_SIZE, "%s", key); // registration
    if (len < 0 || len >= KEY_SIZE) { // todo reset proxy or init proxy after, you have the name, you have to reinit buffer too in this case
        fprintf(stderr, "\033[0;31m[PureMVC::Model::registerProxy] Error: ProxyMap key truncated: '%s' (max %d chars).\033[0m\n", key, KEY_SIZE);
        memset(this->proxyMap[i]->key, 0, KEY_SIZE); // clear
        goto finally;
    }

    registered = true;

finally:
    mutex_unlock(&this->proxyMapMutex);
    return registered;
}

static struct IProxy *retrieveProxy(const struct IModel *self, const char *proxyName) {
    if (proxyName == NULL) return NULL;
    struct Model *this = (struct Model *) self;
    struct IProxy *proxy = NULL;

    mutex_lock_shared(&this->proxyMapMutex);
    for (size_t i = 0; this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->proxyMap[i]->key, proxyName) == 0) {
            proxy = this->proxyMap[i]->proxy;
            break;
        }
    }
    mutex_unlock(&this->proxyMapMutex);

    return proxy;
}

static bool hasProxy(const struct IModel *self, const char *proxyName) {
    if (proxyName == NULL) return false;
    struct Model *this = (struct Model *) self;
    bool exists = false;

    mutex_lock_shared(&this->proxyMapMutex);
    for (size_t i = 0; this->proxyMap != NULL && this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->proxyMap[i]->key, proxyName) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&this->proxyMapMutex);

    return exists;
}

static bool removeProxy(struct IModel *self, const char *proxyName, struct IProxy **out) {
    if (proxyName == NULL) return false;
    struct Model *this = (struct Model *) self;
    bool removed = false;

    mutex_lock(&this->proxyMapMutex);
    for (size_t i = 0, index = 0; this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->proxyMap[i]->key, proxyName) == 0) { // match
            if (out != NULL) *out = this->proxyMap[i]->proxy; // out param

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
    return (sizeof(struct Model) + (sizeof(void *) - 1u)) & ~(sizeof(void *) - 1u);
}

struct IModel *puremvc_model_init(void *buffer, const char *key) {
    struct Model *this = (struct Model *) buffer;

    memset(this, 0, sizeof(struct Model));

    this->base.getMultitonKey = getMultitonKey;
    this->base.initializeModel = initializeModel;
    this->base.registerProxy = registerProxy;
    this->base.retrieveProxy = retrieveProxy;
    this->base.hasProxy = hasProxy;
    this->base.removeProxy = removeProxy;

    int len = snprintf(this->multitonKey, KEY_SIZE, "%s", key);
    if (len < 0 || len >= KEY_SIZE) {
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
    mutex_init(&instanceMapMutex);
}

struct IModel *puremvc_model_getInstance(struct ModelMap **modelMap, const char *key) {
    if (modelMap == NULL || key == NULL) return NULL;
    struct IModel *model = NULL;

    instanceMap = modelMap;

    mutex_once(&mutexOnce, dispatchOnce);
    mutex_lock(&instanceMapMutex);

    size_t i = 0;
    for (; instanceMap != NULL && instanceMap[i] != NULL && instanceMap[i]->key[0] != '\0'; i++) { // find model
        if (strcmp(instanceMap[i]->key, key) == 0) {
            model = instanceMap[i]->model;
            goto finally;
        }
    }

    if (instanceMap == NULL || instanceMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] ERROR: ModelMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        goto finally;
    }

    if (instanceMap[i]->model == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] ERROR: Missing Model storage; skipping registration.\033[0m\n");
        goto finally;
    }

    int len = snprintf(instanceMap[i]->key, KEY_SIZE, "%s", key); // registration
    if (len < 0 || len >= KEY_SIZE) { // todo reset proxy or init proxy after, you have the name
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] Error: ModelMap key truncated: '%s' (max %zu chars).\033[0m\n", key, sizeof(key));
        memset(instanceMap[i]->key, 0, KEY_SIZE);
        goto finally;
    }

    model = puremvc_model_init(instanceMap[i]->model, key); // init

finally:
    mutex_unlock(&instanceMapMutex);
    return model;
}

bool puremvc_model_removeModel(const char *key, struct IModel **out) {
    if (key == NULL) return false;
    bool removed = false;

    if (instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::removeModel] ERROR: Missing ModelMap storage; skipping removal.\033[0m\n");
        return false;
    }

    mutex_once(&mutexOnce, dispatchOnce);
    mutex_lock(&instanceMapMutex);

    size_t index = 0;
    for (size_t i = 0; instanceMap[i] != NULL && instanceMap[i]->key[0] != '\0'; i++) { // find model
        if (strcmp(instanceMap[i]->key, key) == 0) {
            if (out != NULL) *out = instanceMap[i]->model;
            memset(instanceMap[i]->key, 0, KEY_SIZE); // remove
            removed = true;
            // ((struct Model *) s_modelMap[i]->model)->proxyMap[0]->key // todo remove proxies?
        } else {
            if (index != i) { // shift left
                *instanceMap[index] = *instanceMap[i]; // shift left first
                memset(instanceMap[i]->key, 0, KEY_SIZE); // remove
            }
            index++;
        }
    }

    if (index == 0) instanceMap = NULL; // avoid dangling global stack pointer after removal of last entry

    mutex_unlock(&instanceMapMutex);
    return removed;
}
