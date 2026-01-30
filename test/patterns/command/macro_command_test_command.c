#include "macro_command_test_command.h"

#include "macro_command_test_sub1_command.h"
#include "macro_command_test_sub2_command.h"
#include "macro_command_test_command2.h"

static void execute(const struct SimpleCommand *self, struct Notification notification) {
    struct SimpleCommand (*subCommands[MACRO_COMMAND_ARRAY_SIZE])() = {
        macro_command_test_sub1_command,
        macro_command_test_sub2_command,
        macro_command_test_command2,
        NULL
    };

    for (size_t i = 0; subCommands[i] != NULL; i++) {
        struct SimpleCommand (*factory)() = subCommands[i];
        struct SimpleCommand command = factory();
        command.notifier.initializeNotifier(&command.notifier, self->notifier.getMultitonKey(&self->notifier));
        command.execute(&command, notification);
    }
}

struct SimpleCommand macro_command_test_command() {
    struct SimpleCommand command = puremvc_simple_command();
    command.execute = &execute;
    return command;
}
