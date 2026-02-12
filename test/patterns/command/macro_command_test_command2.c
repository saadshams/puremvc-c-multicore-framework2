#include "macro_command_test_command2.h"
#include "macro_command_test_sub3_command.h"

#include "puremvc/platform.h"
#include "puremvc/i_notifier.h"

static bool execute(const struct ICommand *self, struct INotification *notification) {
    if (notification == NULL) return false;

    struct ICommand *(*subCommands[2])(struct ICommand *) = {
        macro_command_test_sub3_command,
        NULL
    };

    for (size_t i = 0; subCommands[i] != NULL; i++) {
        struct ICommand *(*factory)(struct ICommand *) = subCommands[i];
        const struct ICommand *command = factory(alloca(puremvc_macro_command_size()));

        struct INotifier *notifier = command->getNotifier(command); // get multitonKey from the parent macro command
        notifier->initializeNotifier(notifier, self->getNotifier(self)->getMultitonKey(self->getNotifier(self)));

        if (command->execute(command, notification) == false)
            return false;
    }

    return true;
}

struct ICommand *macro_command_test_command2(struct ICommand *const command) {
    struct ICommand *self = puremvc_macro_command_init(command);
    self->execute = execute;
    return self;
}
