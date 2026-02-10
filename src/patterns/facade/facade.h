/**
* @file Facade.h
* @internal
* @brief Facade Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "puremvc/i_facade.h"
#include "puremvc/i_controller.h"
#include "puremvc/i_model.h"
#include "puremvc/i_view.h"

struct Facade {
    struct IFacade base;

    const char *multitonKey;
    struct IController *controller;
    struct IModel *model;
    struct IView *view;
};

