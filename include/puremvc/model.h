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
#include "i_proxy.h"

struct Model {
    struct IModel base;

    char multitonKey[KEY_SIZE];

    Mutex proxyMapMutex;
    struct ProxyMap {
        char key[KEY_SIZE];
        struct IProxy *proxy;
    } **proxyMap;
};

struct ModelMap {
    char key[KEY_SIZE];
    struct IModel *model;
};

struct IModel *puremvc_model_getInstance(struct ModelMap **modelMap, const char *key);

bool puremvc_model_removeModel(const char *key, struct IModel **model);
