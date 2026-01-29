#include "macro_command_test_sub3_command.h"

#include <stdio.h>

#include "macro_command_test_vo.h"

static void execute(const struct SimpleCommand *self, struct Notification *notification) {
    printf("executing sub3 command\n");
    fflush(stdout);
    // struct MacroCommandTestVO *vo = (struct MacroCommandTestVO *) notification->getBody(notification);

    // Fabricate a result
    // vo->result3 = vo->input * vo->input * vo->input;
    printf("finished executing sub3 command\n");
}

struct SimpleCommand macro_command_test_sub3_command() {
    printf("simple sub3 command init\n");
    struct SimpleCommand command = puremvc_simple_command();
    command.notifier.initializeNotifier(&command.notifier, "MacroCommandTestkey1");
    command.execute = execute;
    return command;
}
