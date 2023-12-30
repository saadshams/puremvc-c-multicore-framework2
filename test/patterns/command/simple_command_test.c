#include "simple_command.h"
#include <assert.h>
#include <stdio.h>

typedef struct { int input; int result; } SimpleCommandTestVO;

/**
 * Fabricate a result by multiplying the input by 2
 *
 * @param notification the <code>INotification</code> carrying the <code>SimpleCommandTestVO</code>
 */
static void execute(struct SimpleCommand *command, struct Notification *notification) {
    SimpleCommandTestVO *vo = notification->getBody(notification);

    // Fabricate a result
    vo->result = 2 * vo->input;
}

/**
 * Node Constructor
 */
void testConstructor() {
    struct SimpleCommand *command = puremvc_simple_command_new();

    assert(command != NULL);
    puremvc_simple_command_free(&command);
    assert(command == NULL);
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
    SimpleCommandTestVO vo = {5, 0};

    // Create the Notification (note)
    struct Notification *note = puremvc_notification_new("SimpleCommandTestNote", &vo, NULL);

    // Create the SimpleCommand
    struct SimpleCommand *command = puremvc_simple_command_new();
    command->execute = execute;
    // command->notifier->initializeNotifier(command->notifier, "test");

    // Execute the SimpleCommand
    command->execute(command, note);

    // test assertions
    assert(vo.result == 10);

    puremvc_notification_free(&note);
    assert(note == NULL);
    puremvc_simple_command_free(&command);
    assert(command == NULL);
    // $Facade.removeFacade("test");
}

/**
 * Tests Proxy class destructor method.
 */
void testDestructor() {
    struct SimpleCommand *command = puremvc_simple_command_new();
    assert(command != NULL);

    puremvc_simple_command_free(&command);
    assert(command == NULL);
}

int main() {
    testConstructor();
    testSimpleCommandExecute();
    testDestructor();
    return 0;
}
