/**
* @file simple_command.h
* @brief SimpleCommand Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "notifier.h"
#include "notification.h"

struct SimpleCommand {
    struct Notifier notifier;

    void (*execute)(const struct SimpleCommand *self, struct Notification *notification);
};

struct SimpleCommand puremvc_simple_command();
