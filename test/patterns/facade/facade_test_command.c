#include "facade_test_command.h"
#include "facade_test_vo.h"

void execute(struct SimpleCommand *self, struct Notification *notification) {
    FacadeTestVO *vo = notification->getBody(notification);

    // Fabricate a result
    vo->result = 2 * vo->input;
}

struct SimpleCommand *test_facade_command_new() {
    struct SimpleCommand *command = puremvc_simple_command_new();
    command->execute = execute;
    return command;
}