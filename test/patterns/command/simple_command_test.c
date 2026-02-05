#include <assert.h>

#include "simple_command_test.h"
#include "puremvc/notification.h"
#include "simple_command_test_command.h"
#include "simple_command_test_vo.h"
#include "puremvc/i_notifier.h"

int main() {
    testSimpleCommandExecute();
    return 0;
}

/**
 * Tests the <code>execute</code> method of a <code>SimpleCommand</code>.
 *
 * <P>This test creates a new <code>Notification</code>, adding a
 * <code>SimpleCommandTestVO</code> as the body.
 * It then creates a <code>SimpleCommandTestCommand</code> and invokes
 * its <code>execute</code> method, passing in the note.</P>
 *
 * <P>Success is determined by evaluating a property on the
 * object that was passed on the Notification body, which will
 * be modified by the SimpleCommand</P>.
 */
void testSimpleCommandExecute() {
    // Create the VO
    struct SimpleCommandTestVO vo = {.input = 5};

    // Create the Notification (note)
    struct INotification *notification = puremvc_notification(&(struct Notification){0}, "SimpleCommandTestNote", &vo, NULL);

    struct SimpleCommand *storage = &(struct SimpleCommand) {
        .base = (struct ICommand) { .notifier = &(struct INotifier){0} }
    };

    // Create the SimpleCommand
    struct ICommand *command = test_simple_command(storage);

    // assertions
    assert(storage->base.notifier == command->notifier);

    command->notifier->initializeNotifier(command->notifier, "test");

    // Execute the SimpleCommand
    command->execute(command, notification);


    // test assertions
    assert(vo.result == 10);
}
