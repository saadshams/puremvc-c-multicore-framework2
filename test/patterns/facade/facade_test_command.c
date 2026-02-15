#include "facade_test_command.h"
#include "facade_test_vo.h"

static void execute(const struct ICommand *self, struct INotification *notification) {
    if (notification == NULL) return;

    struct FacadeTestVO *vo = (struct FacadeTestVO *)notification->getBody(notification);

    // Fabricate a result
    vo->result = 2 * vo->input;
}

struct ICommand *test_facade_command(void *buffer) {
    struct ICommand *self = puremvc_simple_command_init(buffer);
    self->execute = execute;
    return self;
}
