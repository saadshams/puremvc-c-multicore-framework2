#include "macro_command_test_command2.h"
#include "macro_command_test_sub3_command.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    struct ICommand *(*subCommands[2])(struct SimpleCommand *) = {
        macro_command_test_sub3_command,
        NULL
    };

    for (size_t i = 0; subCommands[i] != NULL; i++) {
        struct ICommand *(*factory)(struct SimpleCommand *) = subCommands[i];
        const struct ICommand *command = factory(&(struct SimpleCommand){0});
        // command->notifier.base.initializeNotifier(&command.notifier.base, this->notifier.base.getMultitonKey(&this->notifier.base));
        //command.notifier.initializeNotifier(&command.notifier, self->notifier.getMultitonKey(&self->notifier));
        command->execute(command, notification);
    }
}

struct ICommand *macro_command_test_command2(struct SimpleCommand *const simpleCommand) {
    struct ICommand *command = puremvc_simple_command(simpleCommand);
    command->execute = execute;
    return command;
}
