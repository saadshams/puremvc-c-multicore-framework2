#include "macro_command_test_sub2command.h"
#include "macro_command_test_vo.h"

static void execute(struct SimpleCommand *self, struct Notification *notification) {
    MacroCommandTestVO *vo = notification->getBody(notification);

    // Fabricate a result
    vo->result2 = vo->input * vo->input;
}

struct SimpleCommand *macro_command_test_sub2command_new() {
    struct SimpleCommand *simpleCommand = puremvc_simple_command_new();
    simpleCommand->execute = execute;
    return simpleCommand;
}
