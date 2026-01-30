#include "macro_command_test_command2.h"
#include "macro_command_test_sub3_command.h"

static void execute(const struct SimpleCommand *self, struct Notification notification) {
    struct SimpleCommand (*subCommands[MACRO_COMMAND_SUBCOMMANDS_SIZE])() = { macro_command_test_sub3_command };

    for (size_t i = 0; subCommands[i] != NULL; i++) {
        struct SimpleCommand (*factory)() = subCommands[i];
        struct SimpleCommand command = factory();
        command.notifier.initializeNotifier(&command.notifier, self->notifier.getMultitonKey(&self->notifier));
        command.execute(&command, notification);
    }
}

struct SimpleCommand macro_command_test_command2() {
    struct SimpleCommand command = puremvc_simple_command();
    command.execute = execute;
    return command;
}
