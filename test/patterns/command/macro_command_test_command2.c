#include "macro_command_test_command2.h"
#include "macro_command_test_sub3_command.h"
#include "puremvc/i_notifier.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    struct ICommand *(*subCommands[2])(struct SimpleCommand *) = {
        macro_command_test_sub3_command,
        NULL
    };

    for (size_t i = 0; subCommands[i] != NULL; i++) {
        struct ICommand *(*factory)(struct SimpleCommand *) = subCommands[i];
        const struct ICommand *command = factory(&(struct SimpleCommand){0});

        struct INotifier *notifier = command->getNotifier(command); // get multitonKey from the parent macro command
        notifier->initializeNotifier(notifier, self->getNotifier(self)->getMultitonKey(self->getNotifier(self)));

        command->execute(command, notification);
    }
}

struct ICommand *macro_command_test_command2(struct SimpleCommand *const command) {
    puremvc_simple_command(command);
    command->base.execute = execute;
    return &command->base;
}
