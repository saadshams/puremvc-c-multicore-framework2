/**
* @file model.h
* @brief Model Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "puremvc/platform.h"
#include "puremvc/i_model.h"

struct Model {
    struct IModel base;

    char multitonKey[KEY_SIZE];
    struct ProxyMap **proxyMap;

    Mutex proxyMapMutex;
};
