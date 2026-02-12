#include "simple_command_test.h"
#include "simple_command_test_command.h"
#include "simple_command_test_vo.h"

#include "puremvc/i_notifier.h"
#include "puremvc/i_notification.h"

#include <alloca.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void test(const char *name, void (*callback)(void)) {
    printf("\033[0;34m[RUNNING]\033[0m %s...\n", name);
    fflush(stdout);

    callback();

    printf("\033[0;32m[PASSED]\033[0m %s\n", name);
    fflush(stdout);
}

int main() {
    printf("\n\033[1;36m================================================\033[0m\n");
    printf("\033[1;36m[SUITE] %s\033[0m\n", "SimpleCommandTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    test("testSimpleCommandExecute", testSimpleCommandExecute);

    printf("\n\033[1;32m[DONE] All tests in suite finished.\033[0m\n");
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
    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "SimpleCommandTestNote", &vo, NULL);

    // Create the SimpleCommand
    const struct ICommand *command = test_simple_command_init(alloca(puremvc_simple_command_size()));

    // initializer notifier
    command->getNotifier(command)->initializeNotifier(command->getNotifier(command), "testing");

    // Execute the SimpleCommand
    command->execute(command, notification);

    // test assertions
    if (vo.result != 10) abort();
}
