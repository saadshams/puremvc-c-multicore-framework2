#include "macro_command_test_sub3_command.h"
#include "macro_command_test_vo.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    struct MacroCommandTestVO *vo = (struct MacroCommandTestVO *) notification->getBody(notification);

    // Fabricate a result
    vo->result3 = vo->input * vo->input * vo->input;
}

struct ICommand *macro_command_test_sub3_command(struct SimpleCommand *const command) {
    puremvc_simple_command(command);
    command->base.execute = execute;
    return &command->base;
}
