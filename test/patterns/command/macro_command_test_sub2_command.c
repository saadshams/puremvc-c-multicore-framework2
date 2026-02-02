#include "macro_command_test_sub2_command.h"
#include "macro_command_test_vo.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    struct MacroCommandTestVO *vo = (struct MacroCommandTestVO *) notification->getBody(notification);

    // Fabricate a result
    vo->result2 = vo->input * vo->input;
}

struct SimpleCommand macro_command_test_sub2_command() {
    struct SimpleCommand command = puremvc_simple_command();
    command.base.execute = execute;
    return command;
}
