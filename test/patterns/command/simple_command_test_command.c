#include "simple_command_test_command.h"
#include "puremvc/simple_command.h"
#include "simple_command_test_vo.h"
#include "puremvc/notifier.h"

/**
 * Fabricate a result by multiplying the input by 2
 *
 * @param self
 * @param notification the <code>INotification</code> carrying the <code>SimpleCommandTestVO</code>
 */
static void execute(const struct ICommand *self, struct INotification *notification) {
    struct SimpleCommandTestVO *vo = (struct SimpleCommandTestVO *) notification->getBody(notification);

    // Fabricate a result
    vo->result = 2 * vo->input;
}

struct ICommand *test_simple_command(struct SimpleCommand *const command) {
    command->base.execute = execute;
    puremvc_notifier((struct Notifier *) command->base.notifier);
    return &command->base;
}
