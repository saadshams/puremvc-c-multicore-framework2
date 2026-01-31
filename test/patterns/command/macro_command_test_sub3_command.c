#include "macro_command_test_sub3_command.h"
#include "macro_command_test_vo.h"

static void execute(const struct SimpleCommand *self, const struct Notification notification) {
    struct MacroCommandTestVO *vo = (struct MacroCommandTestVO *) notification.getBody(&notification);

    // Fabricate a result
    vo->result3 = vo->input * vo->input * vo->input;
}

struct SimpleCommand macro_command_test_sub3_command() {
    struct SimpleCommand command = puremvc_simple_command();
    command.execute = execute;
    return command;
}
