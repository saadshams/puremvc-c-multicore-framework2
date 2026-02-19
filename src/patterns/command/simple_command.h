/**
* @file simple_command.h
* @brief SimpleCommand Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "puremvc/i_command.h"
#include "patterns/observer/notifier.h"

struct SimpleCommand {
    struct ICommand super;

    struct Notifier notifier;
};
