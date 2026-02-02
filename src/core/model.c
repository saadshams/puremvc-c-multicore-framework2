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

static void initializeModel(struct Model *self) {
    (void)self;
}

static void registerProxy(struct Model *self, struct Proxy proxy) {
    mutex_lock(&self->proxyMapMutex);
    size_t i = 0;
    for (; i < PROXY_MAP_SIZE && self->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->proxyMap[i].key, proxy.base.getName(&proxy.base)) == 0) {
            self->proxyMap[i].proxy.base.onRemove(&self->proxyMap[i].proxy.base);
            memset(&self->proxyMap[i].proxy, 0, sizeof(struct Proxy));
            break;
        }
    }

    if (i >= PROXY_MAP_SIZE) {
        fprintf(stderr, "[PureMVC::Model::registerProxy] Warning: ProxyMap is at capacity for proxy '%s' (max %d proxies); skipping registration.\n", proxy.name, PROXY_MAP_SIZE);
        mutex_unlock(&self->proxyMapMutex);
        return;
    }

    // proxy.notifier.initializeNotifier(&proxy.notifier, self->multitonKey); // todo

    int len = snprintf(self->proxyMap[i].key, KEY_SIZE, "%s", proxy.name);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::Model::registerProxy] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", proxy.name, len, KEY_SIZE);

    self->proxyMap[i].proxy = proxy;
    self->proxyMap[i].proxy.base.onRegister(&self->proxyMap[i].proxy.base);
    mutex_unlock(&self->proxyMapMutex);
}

static struct IProxy *retrieveProxy(struct Model *self, const char *proxyName) {
    mutex_lock_shared(&self->proxyMapMutex);
    for (size_t i = 0; i < PROXY_MAP_SIZE && self->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->proxyMap[i].key, proxyName) == 0) {
            mutex_unlock(&self->proxyMapMutex);
            return &self->proxyMap[i].proxy.base;
        }
    }
    mutex_unlock(&self->proxyMapMutex);
    return NULL;
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
    mutex_unlock(&self->proxyMapMutex);
    return exists;
}

static struct Proxy removeProxy(struct Model *self, const char *proxyName) {
    mutex_lock(&self->proxyMapMutex);
    struct Proxy proxy = {0};
    size_t index = 0;
    for (size_t i = 0; i < PROXY_MAP_SIZE && self->proxyMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->proxyMap[i].key, proxyName) == 0) {
            proxy = self->proxyMap[i].proxy;
            proxy.base.onRemove(&proxy.base);
            memset(&self->proxyMap[index], 0, sizeof(struct ProxyMap));
        } else {
            if (index != i) { // shift left
                memmove(&self->proxyMap[index], &self->proxyMap[i], sizeof(struct ProxyMap));
                memset(&self->proxyMap[i], 0, sizeof(struct ProxyMap));
            }
            index++;
        }
    }

    mutex_unlock(&self->proxyMapMutex);
    return proxy;
}

struct Model puremvc_model(const char *key) {
    struct Model model = {
        .initializeModel = initializeModel,
        .registerProxy = registerProxy,
        .retrieveProxy = retrieveProxy,
        .hasProxy = hasProxy,
        .removeProxy = removeProxy
    };

    int len = snprintf(model.multitonKey, KEY_SIZE, "%s", key);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::Model] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", key, len, KEY_SIZE);

    return model;
}

static void dispatchOnce(void) {
    mutex_init(&modelMapMutex);
}

struct Model *puremvc_model_getInstance(const char *key, struct Model(*factory)(const char *key)) {
    if (key == NULL || factory == NULL) return NULL;
    mutex_once(&token, dispatchOnce);
    mutex_lock(&modelMapMutex);

    size_t i = 0;
    for (; i < INSTANCE_MAP_SIZE && modelMap[i].key[0] != '\0'; i++) {
        if (strncmp(modelMap[i].key, key, KEY_SIZE) == 0) {
            mutex_unlock(&modelMapMutex);
            return &modelMap[i].model;
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

    modelMap[i].model.initializeModel(&modelMap[i].model);

    mutex_unlock(&modelMapMutex);
    return &modelMap[i].model;
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
