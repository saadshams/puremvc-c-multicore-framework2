/**
* @file model.h
* @brief Model Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "mutex.h"
#include "constants.h"
#include "i_model.h"
#include "proxy.h"

struct Model {
    struct IModel base;

    char multitonKey[KEY_SIZE];

    Mutex proxyMapMutex;
    struct ProxyMap {
        char key[KEY_SIZE];
        struct Proxy proxy;
    } proxyMap[PROXY_MAP_SIZE];
};

struct Model puremvc_model(const char *key);

struct IModel *puremvc_model_getInstance(const char *key, struct Model(*factory)(const char *key));

void puremvc_model_removeModel(const char *key);
