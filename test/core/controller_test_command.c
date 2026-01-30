#include "controller_test_command.h"
#include "controller_test_vo.h"

static void execute(const struct SimpleCommand *self, struct Notification notification) {
    struct ControllerTestVO *vo = (struct ControllerTestVO *) notification.getBody(&notification);

    // Fabricate a result
    vo->result = 2 * vo->input;
}

struct SimpleCommand test_controller_command_new() {
    struct SimpleCommand command = puremvc_simple_command();
    command.execute = execute;
    return command;
}
