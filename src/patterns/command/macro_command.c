/**
* @file macro_command.c
* @internal
* @brief MacroCommand Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "macro_command.h"

#include <stddef.h>
#include <stdio.h>

static void execute(const struct ICommand *self, struct INotification *notification) {
    if (notification == NULL) return;

    (void) self;
    struct ICommand *(*subCommands[1])(void) = { NULL };

    for (size_t i = 0; subCommands[i] != NULL; i++) {
        struct ICommand *(*factory)(void) = subCommands[i];
        const struct ICommand *command = factory();

        struct INotifier *notifier = command->getNotifier(command);
        notifier->initializeNotifier(notifier, notifier->getMultitonKey(notifier));

        command->execute(command, notification);
    }
}

size_t puremvc_macro_command_size(void) {
    return puremvc_simple_command_size();
}

struct ICommand *puremvc_macro_command_init(void *buffer) {
    if (buffer == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::MacroCommand::init] Error: Buffer is NULL for MacroCommand - skipping initialization.\033[0m\n");
        return NULL;
    }

    struct ICommand *self = puremvc_simple_command_init(buffer);
    self->execute = execute;
    return self;
}
