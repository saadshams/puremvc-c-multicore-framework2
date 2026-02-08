/**
 * @file i_controller.h
 * @ingroup PureMVC
 *
 * @author Saad Shams <saad.shams@puremvc.org>
 * @copyright BSD 3-Clause License
 */
#pragma once

#include <stdbool.h>

#include "i_command.h"
#include "i_notification.h"

/**
 * @struct IController
 * @ingroup PureMVC
 * @brief IController Interface
 *
 * The Controller maps notifications to command factories and
 * is responsible for executing commands in response to notifications.
 */
struct IController {
    void (*initializeController)(struct IController *self);

    bool (*registerCommand)(struct IController *self, const char *notificationName, struct ICommand *(*factory)());

    void (*executeCommand)(const struct IController *self, struct INotification *notification);

    bool (*hasCommand)(const struct IController *self, const char *notificationName);

    bool (*removeCommand)(struct IController *self, const char *notificationName, struct ICommand *(**factory)());
};
