/**
* @file simple_command.c
* @internal
* @brief SimpleCommand Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/simple_command.h"
#include "puremvc/i_notifier.h"

static struct INotifier *getNotifier(const struct ICommand *self) {
    struct SimpleCommand *this = (struct SimpleCommand *) self;
    return (struct INotifier *) &this->notifier;
}

static void execute(const struct ICommand *self, struct INotification *notification) {
    (void)self; (void)notification;
}

struct ICommand *puremvc_simple_command_init(struct ICommand *const command) {
    struct SimpleCommand *this = (struct SimpleCommand *) command;
    command->getNotifier = getNotifier;
    command->execute = execute;
    puremvc_notifier_init((struct INotifier *) &this->notifier);
    return command;
}
