#include "simple_command_test_command.h"
#include "simple_command_test_vo.h"

/**
 * Fabricate a result by multiplying the input by 2
 *
 * @param self
 * @param notification the <code>INotification</code> carrying the <code>SimpleCommandTestVO</code>
 */
static bool execute(const struct ICommand *self, struct INotification *notification) {
    if (notification == NULL) return false;

    struct SimpleCommandTestVO *vo = (struct SimpleCommandTestVO *) notification->getBody(notification);

    // Fabricate a result
    vo->result = 2 * vo->input;
    return true;
}

struct ICommand *test_simple_command_init(void *buffer) {
    struct ICommand *self = puremvc_simple_command_init(buffer);
    self->execute = execute;
    return self;
}
