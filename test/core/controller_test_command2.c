#include "controller_test_command.h"
#include "controller_test_vo.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    struct ControllerTestVO *vo = (struct ControllerTestVO *)notification->getBody(notification);

    // Fabricate a result
    vo->result = vo->result + (2 * vo->input);
}

struct SimpleCommand test_controller_command2_new() {
    struct SimpleCommand command = puremvc_simple_command();
    command.base.execute = execute;
    return command;
}
