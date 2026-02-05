#include "macro_command_test_command.h"

#include "macro_command_test_sub1_command.h"
#include "macro_command_test_sub2_command.h"
#include "macro_command_test_command2.h"
#include "puremvc/i_notifier.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    struct ICommand *(*subCommands[4])(struct SimpleCommand *) = {
        macro_command_test_sub1_command,
        // macro_command_test_sub2_command,
        // macro_command_test_command2,
        NULL
    };

    struct SimpleCommand subCmds[4] = {0};  // 👈 REAL storage

    for (size_t i = 0; subCommands[i] != NULL; i++) {
        struct ICommand *(*factory)(struct SimpleCommand *) = subCommands[i];
        const struct ICommand *command = factory(&subCmds[i]); // 👈 use stable memory
        // const struct ICommand *command = factory(&(struct SimpleCommand){0});
        // crash
        command->notifier->initializeNotifier(command->notifier, self->notifier->getMultitonKey(self->notifier));
        command->execute(command, notification);
    }
}

struct ICommand *macro_command_test_command(struct SimpleCommand *const simpleCommand) {
    struct ICommand *command = puremvc_simple_command(simpleCommand);
    command->execute = execute;
    return command;
}
