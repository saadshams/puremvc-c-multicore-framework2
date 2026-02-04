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

#include <stdio.h>
#include <string.h>

// mutex for modelMap
static Mutex modelMapMutex;
static MutexOnce token = MUTEX_ONCE_INIT;

static void initializeModel(struct IModel *self) {
    (void)self;
}

static void registerProxy(struct IModel *self, struct Proxy proxy) {
    struct Model *this = (struct Model *) self;

    if (strlen(proxy.name) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Model::registerProxy] Error: Key '%s' too long (max %d) — skipping registration.\n", proxy.name, KEY_SIZE);
        return;
    }

    mutex_lock(&this->proxyMapMutex);

    size_t i = 0;
    for (; this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) { // find existing proxy
        if (strcmp(this->proxyMap[i]->key, proxy.base.getName(&proxy.base)) == 0) {
            this->proxyMap[i]->proxy.base.onRemove(&this->proxyMap[i]->proxy.base);
            memset(&this->proxyMap[i]->proxy, 0, sizeof(struct Proxy));
            break;
        }
    }

    if (this->proxyMap[i] == NULL) { // overflow
        fprintf(stderr, "[PureMVC::Model::registerProxy] Warning: Proxy storage overflow for key '%s' at index %zu; increase slots - skipping registration.\n", proxy.name, i);
        mutex_unlock(&this->proxyMapMutex);
        return;
    }

    proxy.notifier.base.initializeNotifier(&proxy.notifier.base, this->multitonKey);

    snprintf(this->proxyMap[i]->key, KEY_SIZE, "%s", proxy.name); // registration
    this->proxyMap[i]->proxy = proxy;
    this->proxyMap[i]->proxy.base.onRegister(&this->proxyMap[i]->proxy.base);

    mutex_unlock(&this->proxyMapMutex);
}

static struct IProxy *retrieveProxy(struct IModel *self, const char *proxyName) {
    struct Model *this = (struct Model *) self;
    mutex_lock_shared(&this->proxyMapMutex);
    for (size_t i = 0; this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->proxyMap[i]->key, proxyName) == 0) {
            mutex_unlock(&this->proxyMapMutex);
            return &this->proxyMap[i]->proxy.base;
        }
    }
    mutex_unlock(&this->proxyMapMutex);
    return NULL;
}

static bool hasProxy(struct IModel *self, const char *proxyName) {
    struct Model *this = (struct Model *) self;
    mutex_lock_shared(&this->proxyMapMutex);
    bool exists = false;
    for (size_t i = 0; this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) {
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
    struct Proxy proxy = {0};
    size_t index = 0;
    for (size_t i = 0; this->proxyMap[i] != NULL && this->proxyMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->proxyMap[i]->key, proxyName) == 0) {
            proxy = this->proxyMap[i]->proxy;
            proxy.base.onRemove(&proxy.base);

            memset(&this->proxyMap[i]->key, 0, KEY_SIZE);
            this->proxyMap[i]->proxy.base = (struct IProxy){0};
        } else {
            if (index != i) { // shift left
                snprintf(this->proxyMap[index]->key, KEY_SIZE, "%s", this->proxyMap[i]->key);
                this->proxyMap[index]->proxy = this->proxyMap[i]->proxy;

                memset(&this->proxyMap[i]->key, 0, KEY_SIZE);
                this->proxyMap[i]->proxy = (struct Proxy){0};
            }
            index++;
        }
    }

    mutex_unlock(&this->proxyMapMutex);
    return proxy;
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

    model->base.initializeModel(&model->base);
}

static void deinit(struct Model *model) {
    memset(model->multitonKey, 0, KEY_SIZE);
    model->base = (struct IModel){0}; // todo
    mutex_destroy(&model->proxyMapMutex);

    for (size_t j = 0; model->proxyMap && model->proxyMap[j] != NULL; j++) { // clear proxyMap
        memset(model->proxyMap[j]->key, 0, KEY_SIZE);
        model->proxyMap[j]->proxy = (struct Proxy){0};
    }
}

static void dispatchOnce(void) {
    mutex_init(&modelMapMutex);
}

struct IModel *puremvc_model_getInstance(struct ModelMap **modelMap, const char *key) {
    if (key == NULL || modelMap == NULL) return NULL;

    if (strlen(key) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Model::getInstance] Error: Key '%s' too long (max %d) — skipping registration.\n", key, KEY_SIZE);
        return NULL;
    }

    mutex_once(&token, dispatchOnce);
    mutex_lock(&modelMapMutex);

    size_t i = 0;
    for (; modelMap[i] != NULL && modelMap[i]->key[0] != '\0'; i++) { // find model
        if (strcmp(modelMap[i]->key, key) == 0) {
            mutex_unlock(&modelMapMutex);
            return &modelMap[i]->model.base;
        }
    }

    if (modelMap[i] == NULL) { // overflow
        fprintf(stderr, "[PureMVC::Model::getInstance] Warning: Model storage overflow for key '%s' at index %zu; increase slots - skipping registration.\n", key, i);
        mutex_unlock(&modelMapMutex);
        return NULL;
    }

    snprintf(modelMap[i]->key, KEY_SIZE, "%s", key); // init
    init(&modelMap[i]->model, key);

    mutex_unlock(&modelMapMutex);
    return &modelMap[i]->model.base;
}

void puremvc_model_removeModel(struct ModelMap **modelMap, const char *key) {
    if (key == NULL || modelMap == NULL) return;

    if (strlen(key) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Model::removeModel] Error: Key '%s' too long (max %d) — skipping registration.\n", key, KEY_SIZE);
        return;
    }

    mutex_once(&token, dispatchOnce);
    mutex_lock(&modelMapMutex);

    size_t index = 0;
    for (size_t i = 0; modelMap[i] != NULL && modelMap[i]->key[0] != '\0'; i++) { // find model
        if (strncmp(modelMap[i]->key, key, KEY_SIZE) == 0) {
            memset(modelMap[i]->key, 0, KEY_SIZE); // clear model
            deinit(&modelMap[i]->model);
        } else {
            if (index != i) { // shift left
                snprintf(modelMap[index]->key, KEY_SIZE, "%s", modelMap[i]->key); // copy model (destination)
                modelMap[index]->model = modelMap[i]->model;

                memset(modelMap[i]->key, 0, KEY_SIZE); // clear model (source)
                deinit(&modelMap[i]->model);
            }
            index++;
        }
    }
    mutex_unlock(&modelMapMutex);
}
