/**
* @file model.h
* @brief Model Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "puremvc/i_mutex.h"
#include "puremvc/i_model.h"

struct Model {
    struct IModel base;

    const char *multitonKey;
    struct ProxyMap **proxyMap;

    Mutex proxyMapMutex;
};
