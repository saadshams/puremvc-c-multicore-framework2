/**
* @file simple_command.h
* @brief SimpleCommand Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "i_command.h"
#include "notifier.h"

struct SimpleCommand {
    struct ICommand base;

    struct Notifier notifier;
};

struct SimpleCommand puremvc_simple_command();
