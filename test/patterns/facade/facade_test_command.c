#include "facade_test_command.h"
#include "facade_test_vo.h"

static void execute(const struct SimpleCommand *self, struct Notification notification) {
    struct FacadeTestVO *vo = (struct FacadeTestVO *)notification.getBody(&notification);

    // Fabricate a result
    vo->result = 2 * vo->input;
}

struct SimpleCommand test_facade_command() {
    struct SimpleCommand command = puremvc_simple_command();
    command.execute = execute;
    return command;
}
