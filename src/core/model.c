/**
* @file model.c
* @internal
* @brief Model mplementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/mutex.h"
#include "puremvc/model.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

// mutex for modelMap
static Mutex modelMapMutex;
static MutexOnce modelMutexOnce = MUTEX_ONCE_INIT;

static void initializeModel(struct IModel *self) {
    (void)self;
}

static bool registerProxy(struct IModel *self, struct IProxy *(*factory)(struct IProxy *proxy, const char *name, void *data), const char *name, void *data) {
    struct Model *this = (struct Model *) self;

    if (strlen(name) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Model::registerProxy] Error: Key '%s' too long (max %d) — skipping registration.\n", name, KEY_SIZE);
        return false;
    }

    if (this->proxyMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::registerProxy] FATAL: Missing ProxyMap field in ModelMap; skipping registration.\033[0m\n");
        return false;
    }

    mutex_lock(&this->proxyMapMutex);

    size_t i = 0;
    for (; this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) { // find existing
        if (strcmp(this->proxyMap[i]->key, name) == 0) { // match
            this->proxyMap[i]->proxy->onRemove(this->proxyMap[i]->proxy);
            fprintf(stderr, "\033[0;31m[PureMVC::Model::registerMediator] Warning: Proxy '%s' exists; overridden registration\033[0m.\n", name);

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

    // todo check if proxy exists
    struct IProxy *proxy = factory(this->proxyMap[i]->proxy, name, data); // registration
    snprintf(this->proxyMap[i]->key, KEY_SIZE, "%s", proxy->getName(proxy));

    proxy->getNotifier(proxy)->initializeNotifier(proxy->getNotifier(proxy), this->multitonKey);

    mutex_unlock(&this->proxyMapMutex);

    proxy->onRegister(proxy);
    return true;
}

static struct IProxy *retrieveProxy(const struct IModel *self, const char *proxyName) {
    struct Model *this = (struct Model *) self;
    mutex_lock_shared(&this->proxyMapMutex);
    struct IProxy *proxy = NULL;
    for (size_t i = 0; this->proxyMap != NULL && this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->proxyMap[i]->key, proxyName) == 0) {
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
        if (strcmp(this->proxyMap[i]->key, proxyName) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&this->proxyMapMutex);
    return exists;
}

static struct Proxy removeProxy(struct IModel *self, const char *proxyName) {
    struct Model *this = (struct Model *) self;

    mutex_lock(&this->proxyMapMutex);
    struct Proxy result = {0};

    size_t index = 0, i = 0;
    for (; this->proxyMap != NULL && this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->proxyMap[i]->key, proxyName) == 0) { // match
            struct IProxy *proxy = this->proxyMap[i]->proxy;
            proxy->onRemove(proxy);
            memset(&this->proxyMap[i]->key, 0, KEY_SIZE);

            result = *(struct Proxy *)((char *) proxy - offsetof(struct Proxy, base));
        } else {
            if (index != i) { // shift left
                *this->proxyMap[index] = *this->proxyMap[i];
                memset(&this->proxyMap[i]->key, 0, KEY_SIZE);
            }
            index++;
        }
    }

    mutex_unlock(&this->proxyMapMutex);
    return result;
}

static void init(struct Model *model, const char *key) {
    model->base = (struct IModel) {
        .initializeModel = initializeModel,
        .registerProxy = registerProxy,
        .retrieveProxy = retrieveProxy,
        .hasProxy = hasProxy,
        .removeProxy = removeProxy
    };

    snprintf(model->multitonKey, KEY_SIZE, "%s", key);
    mutex_init(&model->proxyMapMutex);
}

static void dispatchOnce(void) {
    mutex_init(&modelMapMutex);
}

struct IModel *puremvc_model_getInstance(struct ModelMap **modelMap, const char *key) {
    if (modelMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] FATAL: Missing ModelMap storage; skipping registration.\033[0m\n");
        return NULL;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] FATAL: Key is NULL; skipping registration.\033[0m\n");
        return NULL;
    }

    if (strlen(key) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Model::getInstance] Error: Key '%s' too long (max %d) — skipping registration.\n", key, KEY_SIZE);
        return NULL;
    }

    mutex_once(&modelMutexOnce, dispatchOnce);
    mutex_lock(&modelMapMutex);

    size_t i = 0;
    for (; modelMap[i] != NULL && modelMap[i]->key[0] != '\0'; i++) { // find model
        if (strncmp(modelMap[i]->key, key, KEY_SIZE) == 0) {
            mutex_unlock(&modelMapMutex);
            return &modelMap[i]->model.base;
        }
    }

    if (modelMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] FATAL: ModelMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        mutex_unlock(&modelMapMutex);
        return NULL;
    }

    snprintf(modelMap[i]->key, KEY_SIZE, "%s", key); // init
    init(&modelMap[i]->model, key);
    modelMap[i]->model.base.initializeModel(&modelMap[i]->model.base);

    mutex_unlock(&modelMapMutex);
    return &modelMap[i]->model.base;
}

void puremvc_model_removeModel(struct ModelMap **modelMap, const char *key) {
    if (modelMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::removeModel] FATAL: Missing ModelMap storage; skipping registration.\033[0m\n");
        return;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::removeModel] FATAL: Key is NULL; skipping registration.\033[0m\n");
        return;
    }

    if (strlen(key) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Model::removeModel] Error: Key '%s' too long (max %d) — skipping registration.\n", key, KEY_SIZE);
        return;
    }

    mutex_once(&modelMutexOnce, dispatchOnce);
    mutex_lock(&modelMapMutex);

    size_t index = 0;
    for (size_t i = 0; modelMap[i] != NULL && modelMap[i]->key[0] != '\0'; i++) { // find model
        if (strncmp(modelMap[i]->key, key, KEY_SIZE) == 0) {
            memset(&modelMap[i]->key, 0, KEY_SIZE); // remove
        } else {
            if (index != i) { // shift left
                *modelMap[index] = *modelMap[i];
                memset(modelMap[i]->key, 0, KEY_SIZE);
            }
            index++;
        }
    }
    mutex_unlock(&modelMapMutex);
}
