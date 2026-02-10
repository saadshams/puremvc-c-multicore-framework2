/**
* @file simple_command.c
* @internal
* @brief SimpleCommand Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "simple_command.h"

#include <string.h>

static struct INotifier *getNotifier(const struct ICommand *self) {
    struct SimpleCommand *this = (struct SimpleCommand *) self;
    return (struct INotifier *) &this->notifier;
}

static void execute(const struct ICommand *self, struct INotification *notification) {
    (void)self; (void)notification;
}

size_t puremvc_simple_command_size() {
    return (sizeof(struct SimpleCommand) + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1);
}

struct ICommand *puremvc_simple_command_init(void *buffer) {
    struct SimpleCommand *this = (struct SimpleCommand *) buffer;

    memset(this, 0, sizeof(struct SimpleCommand));

    this->base.getNotifier = getNotifier;
    this->base.execute = execute;

    puremvc_notifier_init((struct INotifier *) &this->notifier);

    return (struct ICommand *) this;
}
