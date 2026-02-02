/**
* @file simple_command.c
* @internal
* @brief SimpleCommand Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/simple_command.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    (void)self; (void)notification;
}

struct SimpleCommand puremvc_simple_command() {
    return (struct SimpleCommand) {
        .base = {
            .execute = execute
        },
        .notifier = puremvc_notifier(),
    };
}
