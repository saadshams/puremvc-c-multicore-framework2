#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "puremvc/controller.h"

#include "controller_test.h"
#include "controller_test_command.h"
#include "controller_test_command2.h"
#include "controller_test_vo.h"

int main() {
    testGetInstance();
    testRegisterAndExecuteCommand();
    testRegisterAndRemoveCommand();
    testHasCommand();
    testReregisterAndExecuteCommand();
    testRegisterAndUpdateCommand();
    testRemoveController();
    testRegisterAndRemoveMultipleCommands();
    TestViewShiftLeft();
    testCapacityWarning();
    return 0;
}

void testGetInstance() {
    // Test Factory Method
    struct IController *controller = puremvc_controller_getInstance("ControllerTestKey1", puremvc_controller);

    // test assertions
    assert(controller != NULL);
    assert(controller == puremvc_controller_getInstance("ControllerTestKey1", puremvc_controller));
    puremvc_controller_removeController("ControllerTestKey1");
    puremvc_view_removeView("ControllerTestKey1");
    controller = NULL;
}

void testRegisterAndExecuteCommand() {
    // Create the controller, register the ControllerTestCommand to handle 'ControllerTest' notes
    struct IController *controller = puremvc_controller_getInstance("ControllerTestKey2", puremvc_controller);
    controller->registerCommand(controller, "ControllerTest1", test_controller_command_new);

    // Create a 'ControllerTest' note
    struct ControllerTestVO vo = {12, 0};
    struct Notification notification = puremvc_notification("ControllerTest1", &vo, NULL);

    // Tell the controller to execute the Command associated with the note
    // the ControllerTestCommand invoked will multiply the vo.input value
    // by 2 and set the result on vo.result
    controller->executeCommand(controller, &notification.base);

    // test assertions
    assert(vo.result == 24);

    controller->removeCommand(controller, "ControllerTest1");
    puremvc_controller_removeController("ControllerTestKey2");
    puremvc_view_removeView("ControllerTestKey2");
    controller = NULL;
}

void testRegisterAndRemoveCommand() {
    // Create the controller, register the ControllerTestCommand to handle 'ControllerTest' notes
    struct IController *controller = puremvc_controller_getInstance("ControllerTestKey3", puremvc_controller);
    controller->registerCommand(controller, "ControllerRemoveTest", test_controller_command_new);

    // Create a 'ControllerTest' note
    struct ControllerTestVO vo = {12, 0};
    struct Notification notification = puremvc_notification("ControllerRemoveTest", &vo, NULL);

    // Tell the controller to execute the Command associated with the note
    // the ControllerTestCommand invoked will multiply the vo.input value
    // by 2 and set the result on vo.result
    controller->executeCommand(controller, &notification.base);

    // test assertions
    assert(vo.result == 24);

    // Reset result
    vo.result = 0;

    // Remove the Command from the Controller
    controller->removeCommand(controller, "ControllerRemoveTest");

    // Tell the controller to execute the Command associated with the
    // note. This time, it should not be registered, and our vo result
    // will not change
    controller->executeCommand(controller, &notification.base);

    // test assertions
    assert(vo.result == 0);
    puremvc_controller_removeController("ControllerTestKey3");
    puremvc_view_removeView("ControllerTestKey3");
    controller = NULL;
}

void testHasCommand() {
    // register the ControllerTestCommand to handle 'hasCommandTest' notes
    struct IController *controller = puremvc_controller_getInstance("ControllerTestKey4", puremvc_controller);

    // test that hasCommand returns true for hasCommandTest notifications
    controller->registerCommand(controller, "hasCommandTest", test_controller_command_new);
    assert(controller->hasCommand(controller, "hasCommandTest"));

    // Remove the Command from the Controller
    controller->removeCommand(controller, "hasCommandTest");

    // test that hasCommand returns false for hasCommandTest notifications
    assert(controller->hasCommand(controller, "hasCommandTest") == false);

    puremvc_controller_removeController("ControllerTestKey4");
    puremvc_view_removeView("ControllerTestKey4");
    controller = NULL;
}

void testReregisterAndExecuteCommand() {
    // Fetch the controller, register the ControllerTestCommand2 to handle 'ControllerTest2' notes
    struct IController *controller = puremvc_controller_getInstance("ControllerTestKey5", puremvc_controller);
    controller->registerCommand(controller, "ControllerTest2", test_controller_command2_new);

    // Remove the Command from the Controller
    controller->removeCommand(controller, "ControllerTest2");

    // Re-register the Command with the Controller
    controller->registerCommand(controller, "ControllerTest2", test_controller_command2_new);

    // Create a 'ControllerTest2' note
    struct ControllerTestVO vo = {12, 0};
    struct Notification notification = puremvc_notification("ControllerTest2", &vo, NULL);

    // retrieve a reference to the View from the same core.
    struct IView *view = puremvc_view_getInstance("ControllerTestKey5", puremvc_view);
    view->notifyObservers(view, &notification.base);

    // test assertions
    // if the command is executed once the value will be 24
    assert(vo.result == 24);

    // Prove that accumulation works in the VO by sending the notification again
    view->notifyObservers(view, &notification.base);

    // if the command is executed twice the value will be 48
    assert(vo.result == 48);

    controller->removeCommand(controller, "ControllerTest2");
    puremvc_controller_removeController("ControllerTestKey5");
    puremvc_view_removeView("ControllerTestKey5");
    controller = NULL;
}

void testRegisterAndUpdateCommand() {
    struct IController *controller = puremvc_controller_getInstance("ControllerTestKey6", puremvc_controller);

    // first registration
    controller->registerCommand(controller, "ControllerTest2", test_controller_command_new);

    // update command
    controller->registerCommand(controller, "ControllerTest2", test_controller_command2_new);

    struct ControllerTestVO vo = {12, 10};
    struct Notification notification = puremvc_notification("ControllerTest2", &vo, NULL);
    controller->executeCommand(controller, &notification.base);

    // second command result
    assert(vo.result == 34);

    controller->removeCommand(controller, "ControllerTest2");
    puremvc_controller_removeController("ControllerTestKey6");
    puremvc_view_removeView("ControllerTestKey6");
    controller = NULL;
}

void testRemoveController() {
    // Get a Multiton Controller instance
    puremvc_controller_getInstance("ControllerTestKey7", puremvc_controller);

    // remove the controller
    puremvc_controller_removeController("ControllerTestKey7");

    // re-create the controller without throwing an exception
    puremvc_controller("ControllerTestKey7");

    // cleanup
    puremvc_controller_removeController("ControllerTestKey7");
    puremvc_view_removeView("ControllerTestKey7");
}

void testRegisterAndRemoveMultipleCommands() {
    struct IController *controller = puremvc_controller_getInstance("ControllerTestKey8", puremvc_controller);
    struct Controller *self = (struct Controller *) controller;
    struct View *view = (struct View *) self->view;

    struct Notification notification0 = puremvc_notification("command0", NULL, NULL);
    controller->executeCommand(controller, &notification0.base); // crash test
    controller->executeCommand(controller, &notification0.base);

    // Register one command, verify associations and remove it
    controller->registerCommand(controller, "command0", puremvc_simple_command);
    assert(strcmp(self->commandMap[0].key, "command0") == 0);
    assert(self->commandMap[0].factory == puremvc_simple_command);
    assert(strcmp(view->observerMap[0].key, "command0") == 0);
    assert(view->observerMap[0].observers[0].context == controller);
    controller->removeCommand(controller, "command0");
    assert(self->commandMap[0].key[0] == '\0');

    // Register four commands and verify that each is correctly associated to their dictionaries and observers
    controller->registerCommand(controller, "command1", puremvc_simple_command);
    assert(strcmp(self->commandMap[0].key, "command1") == 0);
    assert(self->commandMap[0].factory == puremvc_simple_command);
    assert(strcmp(view->observerMap[0].key, "command1") == 0);
    assert(view->observerMap[0].observers[0].context == controller);

    controller->registerCommand(controller, "command2", puremvc_simple_command);
    assert(strcmp(self->commandMap[1].key, "command2") == 0);
    assert(self->commandMap[1].factory == puremvc_simple_command);
    assert(strcmp(view->observerMap[1].key, "command2") == 0);
    assert(view->observerMap[1].observers[0].context == controller);

    controller->registerCommand(controller, "command3", puremvc_simple_command);
    assert(strcmp(self->commandMap[2].key, "command3") == 0);
    assert(self->commandMap[2].factory == puremvc_simple_command);
    assert(strcmp(view->observerMap[2].key, "command3") == 0);
    assert(view->observerMap[2].observers[0].context == controller);

    controller->registerCommand(controller, "command4", puremvc_simple_command);
    assert(strcmp(self->commandMap[3].key, "command4") == 0);
    assert(self->commandMap[3].factory == puremvc_simple_command);
    assert(strcmp(view->observerMap[3].key, "command4") == 0);
    assert(view->observerMap[3].observers[0].context == controller);

    // Remove the second command (middle) and verify that remaining commands and observers 3, 4 are shifted correctly
    controller->removeCommand(controller, "command2");
    assert(strcmp(self->commandMap[0].key, "command1") == 0);
    assert(self->commandMap[0].factory == puremvc_simple_command);
    assert(strcmp(view->observerMap[1].key, "command3") == 0);
    assert(view->observerMap[1].observers[0].context == controller);
    assert(strcmp(view->observerMap[2].key, "command4") == 0);
    assert(view->observerMap[2].observers[0].context == controller);

    // Remove the last command and verify the remaining command 1, 3 stay in place
    controller->removeCommand(controller, "command4");
    assert(strcmp(self->commandMap[0].key, "command1") == 0);
    assert(self->commandMap[0].factory == puremvc_simple_command);
    assert(strcmp(view->observerMap[1].key, "command3") == 0);
    assert(view->observerMap[1].observers[0].context == controller);

    // Remove the first command and verify that subsequent command 3 shift left
    controller->removeCommand(controller, "command1");
    assert(strcmp(self->commandMap[0].key, "command3") == 0);
    assert(self->commandMap[0].factory == puremvc_simple_command);

    // Remove all remaining mediators and confirm that the dictionary key is cleared
    controller->removeCommand(controller, "command3");
    assert(self->commandMap[0].key[0] == '\0');

    puremvc_controller_removeController("ControllerTestKey8");
    puremvc_view_removeView("ControllerTestKey8");
    controller = NULL;
}

void TestViewShiftLeft() {
    puremvc_controller_getInstance("controller1", puremvc_controller);
    puremvc_controller_getInstance("controller2", puremvc_controller);
    puremvc_controller_getInstance("controller3", puremvc_controller);
    puremvc_controller_getInstance("controller4", puremvc_controller);

    puremvc_controller_removeController("controller2"); // remove middle
    puremvc_controller_removeController("controller4"); // remove last
    puremvc_controller_removeController("controller1"); // remove first
    puremvc_controller_removeController("controller3"); // remove remaining

    puremvc_view_removeView("controller2"); // remove middle
    puremvc_view_removeView("controller4"); // remove last
    puremvc_view_removeView("controller1"); // remove first
    puremvc_view_removeView("controller3"); // remove remaining
}

void testCapacityWarning() {
    for (int i = 0; i < INSTANCE_MAP_SIZE + 1; i++) {
        char key[32] = {0};
        snprintf(key, sizeof(key), "controller%d", i);
        puremvc_controller_getInstance(key, puremvc_controller);
    }

    struct IController *controller = puremvc_controller_getInstance("controller1", puremvc_controller);
    for (int i = 0; i < COMMAND_MAP_SIZE + 1; i++) {
        char key[32] = {0};
        snprintf(key, sizeof(key), "command%d", i);
        controller->registerCommand(controller, key, puremvc_simple_command);
    }
}
