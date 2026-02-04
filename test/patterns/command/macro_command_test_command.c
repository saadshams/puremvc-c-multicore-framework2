#include "macro_command_test_command.h"

#include "macro_command_test_sub1_command.h"
#include "macro_command_test_sub2_command.h"
#include "macro_command_test_command2.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    const struct SimpleCommand *this = (struct SimpleCommand *) self;

    struct SimpleCommand (*subCommands[4])() = {
        macro_command_test_sub1_command,
        macro_command_test_sub2_command,
        macro_command_test_command2,
        NULL
    };

    for (size_t i = 0; subCommands[i] != NULL; i++) {
        struct SimpleCommand (*factory)() = subCommands[i];
        struct SimpleCommand command = factory();
        command.notifier.base.initializeNotifier(&command.notifier.base, this->notifier.base.getMultitonKey(&this->notifier.base));
        // command.notifier.base.initializeNotifier(&command.notifier.base, self->notifier.base.getMultitonKey(&self->notifier.base));
        command.base.execute(&command.base, notification);
    }
}

struct SimpleCommand macro_command_test_command() {
    struct SimpleCommand command = puremvc_simple_command();
    command.base.execute = execute;
    return command;
}
