#include "simple_command_test_command.h"
#include "puremvc/simple_command.h"
#include "simple_command_test_vo.h"

/**
 * Fabricate a result by multiplying the input by 2
 *
 * @param self
 * @param notification the <code>INotification</code> carrying the <code>SimpleCommandTestVO</code>
 */
static void execute(const struct SimpleCommand *self, struct Notification *notification) {
    struct SimpleCommandTestVO *vo = (struct SimpleCommandTestVO *) notification->getBody(notification);

    // Fabricate a result
    vo->result = 2 * vo->input;
}

struct SimpleCommand test_simple_command() {
    struct SimpleCommand command = puremvc_simple_command();
    command.execute = execute;
    return command;
}
