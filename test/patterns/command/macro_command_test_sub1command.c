#include "macro_command_test_sub1command.h"
#include "macro_command_test_vo.h"

static void execute(struct SimpleCommand *self, struct Notification *notification) {
    MacroCommandTestVO *vo = notification->getBody(notification);

    // Fabricate a result
    vo->result1 = 2 * vo->input;
}

struct SimpleCommand *macro_command_test_sub1command_new() {
    struct SimpleCommand *command = puremvc_simple_command_new();
    command->execute = execute;
    return command;
}
