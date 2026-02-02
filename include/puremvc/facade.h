/**
* @file Facade.h
* @internal
* @brief Facade Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "constants.h"
#include "i_facade.h"
#include "i_controller.h"
#include "i_model.h"
#include "i_view.h"

struct Facade {
    struct IFacade base;

    char multitonKey[KEY_SIZE];

    struct IController *controller;
    struct IModel *model;
    struct IView *view;
};

struct Facade puremvc_facade(const char *key);

struct IFacade *puremvc_facade_getInstance(const char *key, struct Facade(*factory)(const char *));

void puremvc_facade_removeFacade(const char *key);

bool puremvc_facade_hasCore(const char *key);
