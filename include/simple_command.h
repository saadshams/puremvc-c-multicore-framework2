#ifndef PUREMVC_SIMPLE_COMMAND_H
#define PUREMVC_SIMPLE_COMMAND_H

#include "notification.h"

struct Notifier;

struct SimpleCommand {
    struct Notifier *notifier;
    void (*execute)(struct SimpleCommand *self, struct Notification *notification);
};

struct SimpleCommand *puremvc_simple_command_init(struct SimpleCommand *self);

struct SimpleCommand *puremvc_simple_command_alloc();

struct SimpleCommand *puremvc_simple_command_new();

void puremvc_simple_command_free(struct SimpleCommand **self);

#endif //PUREMVC_SIMPLE_COMMAND_H
