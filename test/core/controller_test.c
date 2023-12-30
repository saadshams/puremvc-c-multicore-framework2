#include "controller_test.h"
#include "controller_test_command.h"
#include "controller_test_command2.h"
#include "controller_test_vo.h"
#include "controller.h"
#include <assert.h>
#include <stdio.h>

int main() {
    testGetInstance();
    testRegisterAndExecuteCommand();
    testRegisterAndRemoveCommand();
    testHasCommand();
    testReregisterAndExecuteCommand();
    testRegisterAndUpdateCommand();
    testRemoveController();
    return 0;
}

void testGetInstance() {
    // Test Factory Method
    struct Controller *controller = puremvc_controller_getInstance("ControllerTestKey1", puremvc_controller_new);

    // test assertions
    assert(controller != NULL);
    assert(controller == puremvc_controller_getInstance("ControllerTestKey1", puremvc_controller_new));
    puremvc_controller_removeController("ControllerTestKey1");
    controller = NULL;
}

void testRegisterAndExecuteCommand() {
    // Create the controller, register the ControllerTestCommand to handle 'ControllerTest' notes
    struct Controller *controller = puremvc_controller_getInstance("ControllerTestKey2", puremvc_controller_new);
    controller->registerCommand(controller, "ControllerTest1", test_controller_command_new);

    // Create a 'ControllerTest' note
    ControllerTestVO vo = {12, 0};
    struct Notification *notification = puremvc_notification_new("ControllerTest1", &vo, NULL);

    // Tell the controller to execute the Command associated with the note
    // the ControllerTestCommand invoked will multiply the vo.input value
    // by 2 and set the result on vo.result
    controller->executeCommand(controller, notification);

    // test assertions
    assert(vo.result == 24);

    controller->removeCommand(controller, "ControllerTest1");
    puremvc_notification_free(&notification);
    puremvc_controller_removeController("ControllerTestKey2");
    controller = NULL;
}

void testRegisterAndRemoveCommand() {
    // Create the controller, register the ControllerTestCommand to handle 'ControllerTest' notes
    struct Controller *controller = puremvc_controller_getInstance("ControllerTestKey3", puremvc_controller_new);
    controller->registerCommand(controller, "ControllerRemoveTest", test_controller_command_new);

    // Create a 'ControllerTest' note
    ControllerTestVO vo = {12, 0};
    struct Notification *notification = puremvc_notification_new("ControllerRemoveTest", &vo, NULL);

    // Tell the controller to execute the Command associated with the note
    // the ControllerTestCommand invoked will multiply the vo.input value
    // by 2 and set the result on vo.result
    controller->executeCommand(controller, notification);

    // test assertions
    assert(vo.result == 24);

    // Reset result
    vo.result = 0;

    // Remove the Command from the Controller
    controller->removeCommand(controller, "ControllerRemoveTest");

    // Tell the controller to execute the Command associated with the
    // note. This time, it should not be registered, and our vo result
    // will not change
    controller->executeCommand(controller, notification);

    // test assertions
    assert(vo.result == 0);
    puremvc_notification_free(&notification);
    puremvc_controller_removeController("ControllerTestKey3");
    controller = NULL;
}

void testHasCommand() {
    // register the ControllerTestCommand to handle 'hasCommandTest' notes
    struct Controller *controller = puremvc_controller_getInstance("ControllerTestKey4", puremvc_controller_new);

    // test that hasCommand returns true for hasCommandTest notifications
    controller->registerCommand(controller, "hasCommandTest", test_controller_command_new);
    assert(controller->hasCommand(controller, "hasCommandTest"));

    // Remove the Command from the Controller
    controller->removeCommand(controller, "hasCommandTest");

    // test that hasCommand returns false for hasCommandTest notifications
    assert(controller->hasCommand(controller, "hasCommandTest") == false);

    puremvc_controller_removeController("ControllerTestKey4");
    controller = NULL;
}

void testReregisterAndExecuteCommand() {
    // Fetch the controller, register the ControllerTestCommand2 to handle 'ControllerTest2' notes
    struct Controller *controller = puremvc_controller_getInstance("ControllerTestKey5", puremvc_controller_new);
    controller->registerCommand(controller, "ControllerTest2", test_controller_command2_new);

    // Remove the Command from the Controller
    controller->removeCommand(controller, "ControllerTest2");

    // Re-register the Command with the Controller
    controller->registerCommand(controller, "ControllerTest2", test_controller_command2_new);

    // Create a 'ControllerTest2' note
    ControllerTestVO vo = {12, 0};
    struct Notification *notification = puremvc_notification_new("ControllerTest2", &vo, NULL);

    // retrieve a reference to the View from the same core.
    struct View *view = puremvc_view_getInstance("ControllerTestKey5", puremvc_view_new);
    view->notifyObservers(view, notification);

    // test assertions
    // if the command is executed once the value will be 24
    assert(vo.result == 24);

    // Prove that accumulation works in the VO by sending the notification again
    view->notifyObservers(view, notification);

    // if the command is executed twice the value will be 48
    assert(vo.result == 48);

    controller->removeCommand(controller, "ControllerTest2");
    puremvc_notification_free(&notification);
    puremvc_controller_removeController("ControllerTestKey5");
    puremvc_view_removeView("ControllerTestKey5");
    controller = NULL;
}

void testRegisterAndUpdateCommand() {
    struct Controller *controller = puremvc_controller_getInstance("ControllerTestKey3_2", puremvc_controller_new);

    // first registration
    controller->registerCommand(controller, "ControllerTest2", test_controller_command_new);

    // update command
    controller->registerCommand(controller, "ControllerTest2", test_controller_command2_new);

    ControllerTestVO vo = {12, 10};
    struct Notification *notification = puremvc_notification_new("ControllerTest2", &vo, NULL);
    controller->executeCommand(controller, notification);

    // second command result
    assert(vo.result == 34);

    controller->removeCommand(controller, "ControllerTest2");
    puremvc_notification_free(&notification);
    puremvc_controller_removeController("ControllerTestKey3_2");
    controller = NULL;
}

void testRemoveController() {
    // Get a Multiton Controller instance
    puremvc_controller_getInstance("ControllerTestKey4", puremvc_controller_new);

    // remove the controller
    puremvc_controller_removeController("ControllerTestKey4");

    // re-create the controller without throwing an exception
    puremvc_controller_new("ControllerTestKey4");

    // cleanup
    puremvc_controller_removeController("ControllerTestKey4");
}
