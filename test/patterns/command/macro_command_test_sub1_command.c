#include "macro_command_test_sub1_command.h"
#include "macro_command_test_vo.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    struct MacroCommandTestVO *vo = (struct MacroCommandTestVO *) notification->getBody(notification);

    // Fabricate a result
    vo->result1 = 2 * vo->input;
}

struct ICommand *macro_command_test_sub1_command(struct SimpleCommand *const simpleCommand) {
    struct ICommand *command = puremvc_simple_command(simpleCommand);
    command->execute = execute;
    return command;
}
