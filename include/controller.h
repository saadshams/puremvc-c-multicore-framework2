#ifndef PUREMVC_CONTROLLER_H
#define PUREMVC_CONTROLLER_H

#include "view.h"
#include "dictionary.h"
#include "simple_command.h"

struct Controller {

    const char *multitonKey;

    struct View *view;

    Dictionary *commandMap;

    void (*initializeController)(struct Controller *self);

    void (*executeCommand)(struct Controller *self, struct Notification *notification);

    void (*registerCommand)(struct Controller *self, const char *notificationName, struct SimpleCommand *(*factory)(void));

    bool (*hasCommand)(struct Controller *self, const char *notificationName);

    void (*removeCommand)(struct Controller *self, const char *notificationName);
};

struct Controller *puremvc_controller_init(struct Controller *self);

struct Controller *puremvc_controller_alloc(const char *key);

struct Controller *puremvc_controller_new(const char *key);

void puremvc_controller_free(struct Controller **self);

void puremvc_controller_removeController(const char *key);

struct Controller *puremvc_controller_getInstance(const char *key, struct Controller *(*factory)(const char *));

#endif //PUREMVC_CONTROLLER_H
