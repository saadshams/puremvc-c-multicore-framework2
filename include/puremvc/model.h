/**
* @file model.h
* @internal
* @brief Model Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include <stdbool.h>

// #include <mutex.h>
#include "constants.h"
#include "proxy.h"

struct Model {
    char multitonKey[MAX_NAME_LEN];

    // Mutex proxyMapMutex;
    // struct Dictionary *proxyMap;
    struct Proxy proxyMap[MAX_PROXIES];
    size_t proxyCount;
    
    void (*initializeModel)(struct Model *self);

    void (*registerProxy)(struct Model *self, struct Proxy *proxy);

    struct Proxy *(*retrieveProxy)(struct Model *self, const char *proxyName);
    
    bool (*hasProxy)(const struct Model *self, const char *proxyName);
    
    struct Proxy(*removeProxy)(struct Model *self, const char *proxyName);
};

struct Model puremvc_model(const char *key);

struct Model *puremvc_model_getInstance(const char *key, struct Model(*factory)(const char *key));

void puremvc_model_removeModel(const char *key);
