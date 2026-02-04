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

static void execute(const struct ICommand *self, struct INotification *notification) {
    const struct SimpleCommand *this = (struct SimpleCommand *) self;

    struct SimpleCommand (*subCommands[1])() = { NULL };

    for (size_t i = 0; subCommands[i] != NULL; i++) {
        struct SimpleCommand (*factory)() = subCommands[i];
        struct SimpleCommand command = factory();
        command.notifier.base.initializeNotifier(&command.notifier.base, this->notifier.base.getMultitonKey(&this->notifier.base));
        command.base.execute(&command.base, notification);
    }
}

struct ICommand *puremvc_macro_command(struct SimpleCommand *const simple_command) {
    struct ICommand *command = puremvc_simple_command(simple_command);
    command->execute = execute;
    return command;
}
