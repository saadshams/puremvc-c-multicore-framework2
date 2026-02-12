#include "macro_command_test_sub2_command.h"
#include "macro_command_test_vo.h"

static bool execute(const struct ICommand *self, struct INotification *notification) {
    if (notification == NULL) return false;

    struct MacroCommandTestVO *vo = (struct MacroCommandTestVO *) notification->getBody(notification);

    // Fabricate a result
    vo->result2 = vo->input * vo->input;
    return true;
}

struct ICommand *macro_command_test_sub2_command(struct ICommand *const command) {
    struct ICommand *self = puremvc_simple_command_init(command);
    self->execute = execute;
    return self;
}
