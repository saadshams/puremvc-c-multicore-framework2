#include <assert.h>

#include "simple_command_test.h"
#include "simple_command_test_command.h"
#include "simple_command_test_vo.h"

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
    struct Notification note = puremvc_notification("SimpleCommandTestNote", &vo, NULL);

    // Create the SimpleCommand
    const struct SimpleCommand command = test_simple_command();

    // Execute the SimpleCommand
    command.execute(&command, note);

    // test assertions
    assert(vo.result == 10);
}
