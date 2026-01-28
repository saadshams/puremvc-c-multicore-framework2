/**
* @file simple_command.c
* @internal
* @brief SimpleCommand Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/simple_command.h"

static void execute(const struct SimpleCommand *self, struct Notification *notification) {

}

struct SimpleCommand puremvc_simple_command() {
    struct SimpleCommand command = {0};
    command.notifier = puremvc_notifier();
    command.execute = execute;
    return command;
}
