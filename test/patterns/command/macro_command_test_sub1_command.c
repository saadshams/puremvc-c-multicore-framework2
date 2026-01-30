#include "macro_command_test_sub1_command.h"
#include "macro_command_test_vo.h"

static void execute(const struct SimpleCommand *self, struct Notification notification) {
    struct MacroCommandTestVO *vo = (struct MacroCommandTestVO *) notification.getBody(&notification);

    // Fabricate a result
    vo->result1 = 2 * vo->input;
}

struct SimpleCommand macro_command_test_sub1_command() {
    struct SimpleCommand command = puremvc_simple_command();
    command.execute = execute;
    return command;
}
