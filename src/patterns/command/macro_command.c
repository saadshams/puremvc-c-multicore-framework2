/**
* @file macro_command.c
* @internal
* @brief MacroCommand Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include <stddef.h>

#include "puremvc/macro_command.h"
#include "puremvc/constants.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    const struct SimpleCommand *this = (struct SimpleCommand *) self;

    struct SimpleCommand (*subCommands[MACRO_COMMAND_ARRAY_SIZE])() = { NULL };

    for (size_t i = 0; subCommands[i] != NULL; i++) {
        struct SimpleCommand (*factory)() = subCommands[i];
        struct SimpleCommand command = factory();
        command.notifier.base.initializeNotifier(&command.notifier.base, this->notifier.base.getMultitonKey(&this->notifier.base));
        // command.notifier.initializeNotifier(&command.notifier, self->notifier.getMultitonKey(&self->notifier));
        command.base.execute(&command.base, notification);
    }
}

struct SimpleCommand puremvc_macro_command() {
    struct SimpleCommand command = puremvc_simple_command();
    command.base.execute = execute;
    return command;
}
