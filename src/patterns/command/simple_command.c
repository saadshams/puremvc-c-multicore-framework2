/**
* @file simple_command.c
* @internal
* @brief SimpleCommand Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/simple_command.h"
#include "puremvc/notifier.h"

static struct INotifier *getNotifier(const struct ICommand *self) {
    struct SimpleCommand *this = (struct SimpleCommand *) self;
    return &this->notifier.base;
}

static void execute(const struct ICommand *self, struct INotification *notification) {
    (void)self; (void)notification;
}

struct ICommand *puremvc_simple_command_init(struct SimpleCommand *const command) {
    command->base.getNotifier = getNotifier;
    command->base.execute = execute;
    puremvc_notifier_init(&command->notifier);
    return &command->base;
}

void puremvc_simple_command_deinit(struct SimpleCommand *command) {

}
