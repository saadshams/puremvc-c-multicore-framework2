/**
* @file macro_command.c
* @internal
* @brief MacroCommand Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include <stdio.h>

#include "puremvc/macro_command.h"

static void initializeMacroCommand(struct MacroCommand *self) {

}

static void addSubCommand(struct MacroCommand *self, struct SimpleCommand(*factory)()) {
    if (self->count >= MACRO_COMMAND_ARRAY_SIZE) return;
    self->subCommands[self->count] = factory;
    self->count++;
}

static void execute(struct MacroCommand *self, struct Notification *notification) { // a macro command runs command collection, or it can run another macro command (collection of commands)
    printf("macro command executing\n");
    fflush(stdout);
    self->initializeMacroCommand(self);

    for (size_t i = 0; i < self->count; i++) {
        struct SimpleCommand (*factory)() = self->subCommands[i];
        struct SimpleCommand command = factory(); // crashes or unit test fails here
        command.notifier.initializeNotifier(&command.notifier, "MacroCommandTestkey1");
        command.execute(&command, notification);
    }

    self->count = 0;
    printf("end macro command execution\n");
}

struct MacroCommand puremvc_macro_command() {
    struct MacroCommand command = {0};

    command.simple_command = puremvc_simple_command();
    command.notifier = puremvc_notifier();

    command.initializeMacroCommand = initializeMacroCommand;
    command.addSubCommand = addSubCommand;
    command.simple_command.execute = (void (*)(const struct SimpleCommand *, struct Notification *)) execute;
    command.execute = (void (*)(const struct MacroCommand *, struct Notification *)) execute;
    return command;
}
