/**
* @file macro_command.c
* @internal
* @brief MacroCommand Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/macro_command.h"

#include <stddef.h>

#include "puremvc/i_notifier.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    struct ICommand *(*subCommands[1])() = { NULL };

    for (size_t i = 0; subCommands[i] != NULL; i++) {
        struct ICommand *(*factory)() = subCommands[i];
        const struct ICommand *command = factory();
        command->notifier->initializeNotifier(command->notifier, self->notifier->getMultitonKey(self->notifier));
        command->execute(command, notification);
    }
}

struct ICommand *puremvc_macro_command(struct SimpleCommand *const simple_command) {
    struct ICommand *command = puremvc_simple_command(simple_command);
    command->execute = execute;
    return command;
}
