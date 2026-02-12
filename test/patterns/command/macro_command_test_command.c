#include "macro_command_test_command.h"
#include "macro_command_test_sub1_command.h"
#include "macro_command_test_sub2_command.h"
#include "macro_command_test_command2.h"

#include "puremvc/platform.h"
#include "puremvc/i_notifier.h"

static bool execute(const struct ICommand *self, struct INotification *notification) {
    if (notification == NULL) return false;

    struct ICommand *(*subCommands[4])(struct ICommand *) = {
        macro_command_test_sub1_command,
        macro_command_test_sub2_command,
        macro_command_test_command2, // nested macro command
        NULL
    };

    for (size_t i = 0; subCommands[i] != NULL; i++) {
        struct ICommand *(*factory)(struct ICommand *) = subCommands[i];
        const struct ICommand *command = factory(alloca(puremvc_simple_command_size()));

        struct INotifier *notifier = command->getNotifier(command); // get multitonKey from the parent macro command
        notifier->initializeNotifier(notifier, self->getNotifier(self)->getMultitonKey(self->getNotifier(self)));

        if (command->execute(command, notification) == false) // stop execution if subCommands fail
            return false;
    }

    return true;
}

struct ICommand *macro_command_test_command(void *buffer) {
    struct ICommand *self = puremvc_macro_command_init(buffer);
    self->execute = execute;
    return self;
}
