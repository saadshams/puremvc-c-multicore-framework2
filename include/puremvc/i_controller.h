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
#include "i_view.h"

#ifndef KEY_SIZE
#define KEY_SIZE 32
#endif

struct ControllerMap {
    char key[KEY_SIZE];
    struct IController *controller;
};

struct CommandMap {
    char key[KEY_SIZE];
    struct ICommand *(*factory)(void *);
};

/**
 * @struct IController
 * @ingroup PureMVC
 * @brief IController Interface
 *
 * The Controller maps notifications to command factories and
 * is responsible for executing commands in response to notifications.
 */
struct IController {
    void (*initializeController)(struct IController *self, struct IView *view, struct CommandMap **commandMap);

    bool (*registerCommand)(struct IController *self, const char *notificationName, struct ICommand *(*factory)(void *buffer));

    void (*executeCommand)(const struct IController *self, struct INotification *notification);

    bool (*hasCommand)(const struct IController *self, const char *notificationName);

    bool (*removeCommand)(struct IController *self, const char *notificationName, struct ICommand *(**out)(void *buffer));
};

size_t puremvc_controller_size();

struct IController *puremvc_controller_getInstance(struct ControllerMap **controllerMap, const char *key);

bool puremvc_controller_removeController(const char *key, struct IController **out);
