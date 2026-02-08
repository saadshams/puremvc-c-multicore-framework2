/**
* @file model.c
* @internal
* @brief Model mplementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/model.h"
#include "puremvc/proxy.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

// modelMap
static struct ModelMap **s_modelMap = NULL;

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

static bool removeProxy(struct IModel *self, const char *proxyName, struct IProxy **proxy) {
    struct Model *this = (struct Model *) self;
    bool removed = false;

    mutex_lock(&this->proxyMapMutex);

    size_t index = 0, i = 0;
    for (; this->proxyMap != NULL && this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->proxyMap[i]->key, proxyName) == 0) { // match
            if (proxy != NULL)
                *proxy = this->proxyMap[i]->proxy;

            this->proxyMap[i]->proxy->onRemove(this->proxyMap[i]->proxy);

            memset(&this->proxyMap[i]->key, 0, KEY_SIZE); // remove
            removed = true;
        } else {
            if (index != i) { // shift left
                *this->proxyMap[index] = *this->proxyMap[i];
                memset(&this->proxyMap[i]->key, 0, KEY_SIZE);
            }
            index++;
        }
    }

    mutex_unlock(&this->proxyMapMutex);
    return removed;
}

static void puremvc_model_init(struct IModel *model, const char *key) {
    struct Model *this = (struct Model *) model;

    model->initializeModel = initializeModel;
    model->registerProxy = registerProxy;
    model->retrieveProxy = retrieveProxy;
    model->hasProxy = hasProxy;
    model->removeProxy = removeProxy;

    snprintf(this->multitonKey, KEY_SIZE, "%s", key);
    mutex_init(&this->proxyMapMutex);
}

static void dispatchOnce(void) {
    mutex_init(&modelMapMutex);
}

struct IModel *puremvc_model_getInstance(struct ModelMap **modelMap, const char *key) {
    if (modelMap == NULL && s_modelMap == NULL) {
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

    if (s_modelMap == NULL)
        s_modelMap = modelMap;

    mutex_once(&modelMutexOnce, dispatchOnce);
    mutex_lock(&modelMapMutex);

    size_t i = 0;
    for (; s_modelMap[i] != NULL && s_modelMap[i]->key[0] != '\0'; i++) { // find model
        if (strncmp(s_modelMap[i]->key, key, KEY_SIZE) == 0) {
            mutex_unlock(&modelMapMutex);
            return s_modelMap[i]->model;
        }
    }

    if (s_modelMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] FATAL: ModelMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        mutex_unlock(&modelMapMutex);
        return NULL;
    }

    if (s_modelMap[i]->model == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] FATAL: Missing Model storage; skipping registration.\033[0m\n");
        return NULL;
    }

    snprintf(s_modelMap[i]->key, KEY_SIZE, "%s", key); // init
    puremvc_model_init(s_modelMap[i]->model, key);
    s_modelMap[i]->model->initializeModel(s_modelMap[i]->model);

    mutex_unlock(&modelMapMutex);
    return s_modelMap[i]->model;
}

bool puremvc_model_removeModel(const char *key, struct IModel **model) {
    if (s_modelMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::removeModel] FATAL: Missing ModelMap storage; skipping removal.\033[0m\n");
        return false;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::removeModel] FATAL: Key is NULL; skipping removal.\033[0m\n");
        return false;
    }

    if (strlen(key) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Model::removeModel] Error: Key '%s' too long (max %d) — skipping removal.\n", key, KEY_SIZE);
        return false;
    }

    mutex_once(&modelMutexOnce, dispatchOnce);
    mutex_lock(&modelMapMutex);

    size_t index = 0;
    for (size_t i = 0; s_modelMap[i] != NULL && s_modelMap[i]->key[0] != '\0'; i++) { // find model
        if (strncmp(s_modelMap[i]->key, key, KEY_SIZE) == 0) {
            memset(s_modelMap[i]->key, 0, KEY_SIZE); // remove
            if (model != NULL)
                *model = s_modelMap[i]->model;
        } else {
            if (index != i) { // shift left
                *s_modelMap[index] = *s_modelMap[i];
                memset(s_modelMap[i]->key, 0, KEY_SIZE);
            }
            index++;
        }
    }

    if (index == 0) // all keys were removed; reset
        s_modelMap = NULL;

    mutex_unlock(&modelMapMutex);

    return true;
}
