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

// The Multiton modelMap.
static struct ModelMap {
    char key[KEY_SIZE];
    struct Model model;
} modelMap[INSTANCE_MAP_SIZE];

// mutex for modelMap
static Mutex modelMapMutex;
static MutexOnce token = MUTEX_ONCE_INIT;

static void initializeModel(struct IModel *self) {
    (void)self;
}

static void registerProxy(struct IModel *self, struct Proxy proxy) {
    struct Model *this = (struct Model *) self;
    mutex_lock(&this->proxyMapMutex);
    size_t i = 0;
    for (; i < PROXY_MAP_SIZE && this->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->proxyMap[i].key, proxy.base.getName(&proxy.base)) == 0) {
            this->proxyMap[i].proxy.base.onRemove(&this->proxyMap[i].proxy.base);
            memset(&this->proxyMap[i].proxy, 0, sizeof(struct Proxy));
            break;
        }
    }

    if (i >= PROXY_MAP_SIZE) {
        fprintf(stderr, "[PureMVC::Model::registerProxy] Warning: ProxyMap is at capacity for proxy '%s' (max %d proxies); skipping registration.\n", proxy.name, PROXY_MAP_SIZE);
        mutex_unlock(&this->proxyMapMutex);
        return;
    }

    // proxy.notifier.initializeNotifier(&proxy.notifier, this->multitonKey); // todo

    int len = snprintf(this->proxyMap[i].key, KEY_SIZE, "%s", proxy.name);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::Model::registerProxy] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", proxy.name, len, KEY_SIZE);

    this->proxyMap[i].proxy = proxy;
    this->proxyMap[i].proxy.base.onRegister(&this->proxyMap[i].proxy.base);
    mutex_unlock(&this->proxyMapMutex);
}

static struct IProxy *retrieveProxy(struct IModel *self, const char *proxyName) {
    struct Model *this = (struct Model *) self;
    mutex_lock_shared(&this->proxyMapMutex);
    for (size_t i = 0; i < PROXY_MAP_SIZE && this->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->proxyMap[i].key, proxyName) == 0) {
            mutex_unlock(&this->proxyMapMutex);
            return &this->proxyMap[i].proxy.base;
        }
    }
    mutex_unlock(&this->proxyMapMutex);
    return NULL;
}

static bool hasProxy(struct IModel *self, const char *proxyName) {
    struct Model *this = (struct Model *) self;
    mutex_lock_shared(&this->proxyMapMutex);
    bool exists = false;
    for (size_t i = 0; i < PROXY_MAP_SIZE && this->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->proxyMap[i].key, proxyName) == 0) {
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
    for (size_t i = 0; i < PROXY_MAP_SIZE && this->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->proxyMap[i].key, proxyName) == 0) {
            proxy = this->proxyMap[i].proxy;
            proxy.base.onRemove(&proxy.base);
            memset(&this->proxyMap[index], 0, sizeof(struct ProxyMap));
        } else {
            if (index != i) { // shift left
                memmove(&this->proxyMap[index], &this->proxyMap[i], sizeof(struct ProxyMap));
                memset(&this->proxyMap[i], 0, sizeof(struct ProxyMap));
            }
            index++;
        }
    }

    mutex_unlock(&this->proxyMapMutex);
    return proxy;
}

struct Model puremvc_model(const char *key) {
    struct Model model = {
        .base = {
            .initializeModel = initializeModel,
            .registerProxy = registerProxy,
            .retrieveProxy = retrieveProxy,
            .hasProxy = hasProxy,
            .removeProxy = removeProxy
        }
    };

    int len = snprintf(model.multitonKey, KEY_SIZE, "%s", key);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::Model] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", key, len, KEY_SIZE);

    return model;
}

static void dispatchOnce(void) {
    mutex_init(&modelMapMutex);
}

struct IModel *puremvc_model_getInstance(const char *key, struct Model(*factory)(const char *key)) {
    if (key == NULL || factory == NULL) return NULL;
    mutex_once(&token, dispatchOnce);
    mutex_lock(&modelMapMutex);

    size_t i = 0;
    for (; i < INSTANCE_MAP_SIZE && modelMap[i].key[0] != '\0'; i++) {
        if (strncmp(modelMap[i].key, key, KEY_SIZE) == 0) {
            mutex_unlock(&modelMapMutex);
            return &modelMap[i].model.base;
        }
    }

    if (i >= INSTANCE_MAP_SIZE) {
        fprintf(stderr, "[PureMVC::Model::getInstance] Warning: InstanceMap is at capacity for key '%s' (max %d instances); skipping registration.\n", key, INSTANCE_MAP_SIZE);
        mutex_unlock(&modelMapMutex);
        return NULL;
    }

    int len = snprintf(modelMap[i].key, KEY_SIZE, "%s", key);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::Model::getInstance] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", key, len, KEY_SIZE);

    modelMap[i].model = factory(key);
    mutex_init(&modelMap[i].model.proxyMapMutex);

    modelMap[i].model.base.initializeModel(&modelMap[i].model.base);

    mutex_unlock(&modelMapMutex);
    return &modelMap[i].model.base;
}

void puremvc_model_removeModel(const char *key) {
    if (key == NULL) return;
    mutex_once(&token, dispatchOnce);
    mutex_lock(&modelMapMutex);

    size_t index = 0;
    for (size_t i = 0; i < INSTANCE_MAP_SIZE && modelMap[i].key[0] != '\0'; i++) {
        if (strcmp(modelMap[i].key, key) == 0) {
            memset(&modelMap[i], 0, sizeof(struct ModelMap));
        } else {
            if (index != i) {
                memmove(&modelMap[index], &modelMap[i], sizeof(struct ModelMap));
                memset(&modelMap[i], 0, sizeof(struct ModelMap));
            }
            index++;
        }
    }
    mutex_unlock(&modelMapMutex);
}
