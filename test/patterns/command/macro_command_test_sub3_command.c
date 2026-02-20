#include "macro_command_test_sub3_command.h"
#include "macro_command_test_vo.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    if (notification == NULL) return;

    struct MacroCommandTestVO *vo = notification->getBody(notification);

    // Fabricate a result
    vo->result3 = vo->input * vo->input * vo->input;
}

struct ICommand *macro_command_test_sub3_command(struct ICommand *const command) {
    struct ICommand *self = puremvc_simple_command_init(command);
    self->execute = execute;
    return self;
}
