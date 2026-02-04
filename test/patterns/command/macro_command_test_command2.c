#include "macro_command_test_command2.h"
#include "macro_command_test_sub3_command.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    const struct SimpleCommand *this = (struct SimpleCommand *) self;

    struct SimpleCommand (*subCommands[2])() = { macro_command_test_sub3_command, NULL };

    for (size_t i = 0; subCommands[i] != NULL; i++) {
        struct SimpleCommand (*factory)() = subCommands[i];
        struct SimpleCommand command = factory();
        command.notifier.base.initializeNotifier(&command.notifier.base, this->notifier.base.getMultitonKey(&this->notifier.base));
        //command.notifier.initializeNotifier(&command.notifier, self->notifier.getMultitonKey(&self->notifier));
        command.base.execute(&command.base, notification);
    }
}

struct SimpleCommand macro_command_test_command2() {
    struct SimpleCommand command = puremvc_simple_command();
    command.base.execute = execute;
    return command;
}
