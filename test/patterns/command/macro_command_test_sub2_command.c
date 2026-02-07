#include "macro_command_test_sub2_command.h"
#include "macro_command_test_vo.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    struct MacroCommandTestVO *vo = (struct MacroCommandTestVO *) notification->getBody(notification);

    // Fabricate a result
    vo->result2 = vo->input * vo->input;
}

struct ICommand *macro_command_test_sub2_command(struct ICommand *const command) {
    struct ICommand *self = puremvc_simple_command_init(command);
    self->execute = execute;
    return self;
}
