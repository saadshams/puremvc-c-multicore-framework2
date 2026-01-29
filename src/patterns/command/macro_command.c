/**
* @file macro_command.c
* @brief MacroCommand Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include <stdio.h>

#include "puremvc/macro_command.h"
#include "puremvc/constants.h"

static void execute(const struct SimpleCommand *self, struct Notification *notification) {
    struct SimpleCommand (*subCommands[MACRO_COMMAND_ARRAY_SIZE])() = { NULL };

    for (size_t i = 0; subCommands[i] != NULL; i++) {
        struct SimpleCommand (*factory)() = subCommands[i];
        struct SimpleCommand command = factory();
        command.notifier.initializeNotifier(&command.notifier, self->notifier.getMultitonKey(&self->notifier));
        command.execute(&command, notification);
    }
}

struct SimpleCommand puremvc_macro_command() {
    struct SimpleCommand command = puremvc_simple_command();
    command.execute = execute;
    return command;
}
