#include "controller_test_command.h"
#include "controller_test_vo.h"

static void execute(struct SimpleCommand *self, struct Notification *notification) {
    ControllerTestVO *vo = notification->getBody(notification);

    // Fabricate a result
    vo->result = vo->result + (2 * vo->input);
}

struct SimpleCommand *test_controller_command2_new() {
    struct SimpleCommand *command = puremvc_simple_command_new();
    command->execute = execute;
    return command;
}