/**
* @file simple_command.c
* @internal
* @brief SimpleCommand Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/simple_command.h"

static void execute(const struct SimpleCommand *self, struct Notification notification) {
    (void)self;
    (void)notification;
}

struct SimpleCommand puremvc_simple_command() {
    return (struct SimpleCommand) {
        .notifier = puremvc_notifier(),
        .execute = execute
    };
}
