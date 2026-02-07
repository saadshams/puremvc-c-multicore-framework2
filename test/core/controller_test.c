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
    return 0;
}

void testGetInstance() {
    struct ControllerMap *storage[] = { &(struct ControllerMap) { // supply empty key storage
        .key = "",
        .controller = { .commandMap = (struct CommandMap *[]){} },
    }, NULL };

    // Test Factory Method
    struct IController *controller = puremvc_controller_getInstance(storage, "ControllerTestKey1");

    // test assertions
    assert(controller != NULL);
    assert(controller == puremvc_controller_getInstance(storage, "ControllerTestKey1"));
    puremvc_controller_removeController(storage, "ControllerTestKey1");
    // puremvc_view_removeView(storage, "ControllerTestKey1"); // todo fix view
    controller = NULL;
}

void testRegisterAndExecuteCommand() {
    struct ControllerMap *storage[] = { &(struct ControllerMap) { // supply empty key storage
        .key = "",
        .controller = { .commandMap = (struct CommandMap *[]){} },
    }, NULL };

    // Create the controller, register the ControllerTestCommand to handle 'ControllerTest' notes
    struct IController *controller = puremvc_controller_getInstance(storage, "ControllerTestKey2");
    controller->registerCommand(controller, "ControllerTest1", test_controller_command_new);

    // Create a 'ControllerTest' note
    struct ControllerTestVO vo = {12, 0};
    struct INotification *notification = puremvc_notification_init(&(struct Notification){0}.base, "ControllerTest1", &vo, NULL);

    // Tell the controller to execute the Command associated with the note
    // the ControllerTestCommand invoked will multiply the vo.input value
    // by 2 and set the result on vo.result
    controller->executeCommand(controller, notification);

    // test assertions
    assert(vo.result == 24);

    struct ICommand *(**factory)() = 0;
    controller->removeCommand(controller, factory, "ControllerTest1");
    puremvc_controller_removeController(storage, "ControllerTestKey2");
    // puremvc_view_removeView(controller, "ControllerTestKey2"); // todo provide viewMap
    controller = NULL;
}

void testRegisterAndRemoveCommand() {
    struct ControllerMap *storage[] = { &(struct ControllerMap) { // supply empty key storage
        .key = "",
        .controller = { .commandMap = (struct CommandMap *[]){} },
    }, NULL };

    // Create the controller, register the ControllerTestCommand to handle 'ControllerTest' notes
    struct IController *controller = puremvc_controller_getInstance(storage, "ControllerTestKey3");
    controller->registerCommand(controller, "ControllerRemoveTest", test_controller_command_new);

    // Create a 'ControllerTest' note
    struct ControllerTestVO vo = {12, 0};
    struct INotification *notification = puremvc_notification_init(&(struct Notification){0}.base, "ControllerRemoveTest", &vo, NULL);

    // Tell the controller to execute the Command associated with the note
    // the ControllerTestCommand invoked will multiply the vo.input value
    // by 2 and set the result on vo.result
    controller->executeCommand(controller, notification);

    // test assertions
    assert(vo.result == 24);

    // Reset result
    vo.result = 0;

    // Remove the Command from the Controller
    controller->removeCommand(controller, NULL, "ControllerRemoveTest");

    // Tell the controller to execute the Command associated with the
    // note. This time, it should not be registered, and our vo result
    // will not change
    controller->executeCommand(controller, notification);

    // test assertions
    assert(vo.result == 0);
    puremvc_controller_removeController(storage, "ControllerTestKey3");
    // puremvc_view_removeView("ControllerTestKey3");
    controller = NULL;
}

void testHasCommand() {
    struct ControllerMap *storage[] = { &(struct ControllerMap) { // supply empty key storage
        .key = "",
        .controller = { .commandMap = (struct CommandMap *[]){} },
    }, NULL };

    // register the ControllerTestCommand to handle 'hasCommandTest' notes
    struct IController *controller = puremvc_controller_getInstance(storage, "ControllerTestKey4");

    // test that hasCommand returns true for hasCommandTest notifications
    controller->registerCommand(controller, "hasCommandTest", test_controller_command_new);
    assert(controller->hasCommand(controller, "hasCommandTest"));

    // Remove the Command from the Controller
    controller->removeCommand(controller, NULL, "hasCommandTest");

    // test that hasCommand returns false for hasCommandTest notifications
    assert(controller->hasCommand(controller, "hasCommandTest") == false);

    puremvc_controller_removeController(storage, "ControllerTestKey4");
    // puremvc_view_removeView("ControllerTestKey4");
    controller = NULL;
}

void testReregisterAndExecuteCommand() {
    struct ControllerMap *storage[] = { &(struct ControllerMap) { // supply empty key storage
        .key = "",
        .controller = { .commandMap = (struct CommandMap *[]){} },
    }, NULL };

    // Fetch the controller, register the ControllerTestCommand2 to handle 'ControllerTest2' notes
    struct IController *controller = puremvc_controller_getInstance(storage, "ControllerTestKey5");
    controller->registerCommand(controller, "ControllerTest2", test_controller_command2_new);

    // Remove the Command from the Controller
    controller->removeCommand(controller, NULL, "ControllerTest2");

    // Re-register the Command with the Controller
    controller->registerCommand(controller, "ControllerTest2", test_controller_command2_new);

    // Create a 'ControllerTest2' note
    struct ControllerTestVO vo = {12, 0};
    struct INotification *notification = puremvc_notification_init(&(struct Notification){0}.base, "ControllerTest2", &vo, NULL);

    // retrieve a reference to the View from the same core.
    // struct IView *view = puremvc_view_getInstance(storage, "ControllerTestKey5");
    // view->notifyObservers(view, notification);

    // test assertions
    // if the command is executed once the value will be 24
    assert(vo.result == 24);

    // Prove that accumulation works in the VO by sending the notification again
    // view->notifyObservers(view, notification);

    // if the command is executed twice the value will be 48
    assert(vo.result == 48);

    controller->removeCommand(controller, NULL, "ControllerTest2");
    puremvc_controller_removeController(storage, "ControllerTestKey5");
    // puremvc_view_removeView("ControllerTestKey5");
    controller = NULL;
}

void testRegisterAndUpdateCommand() {
    struct ControllerMap *storage[] = { &(struct ControllerMap) { // supply empty key storage
        .key = "",
        .controller = { .commandMap = (struct CommandMap *[]){} },
    }, NULL };

    struct IController *controller = puremvc_controller_getInstance(storage, "ControllerTestKey6");

    // first registration
    controller->registerCommand(controller, "ControllerTest2", test_controller_command_new);

    // update command
    controller->registerCommand(controller, "ControllerTest2", test_controller_command2_new);

    struct ControllerTestVO vo = {12, 10};
    struct INotification *notification = puremvc_notification_init(&(struct Notification){0}.base, "ControllerTest2", &vo, NULL);
    controller->executeCommand(controller, notification);

    // second command result
    assert(vo.result == 34);

    controller->removeCommand(controller, NULL, "ControllerTest2");
    puremvc_controller_removeController(storage, "ControllerTestKey6");
    // puremvc_view_removeView("ControllerTestKey6");
    controller = NULL;
}

void testRemoveController() {
    struct ControllerMap *storage[] = { &(struct ControllerMap) { // supply empty key storage
        .key = "",
        .controller = { .commandMap = (struct CommandMap *[]){} },
    }, NULL };

    // Get a Multiton Controller instance
    puremvc_controller_getInstance(storage, "ControllerTestKey7");

    // remove the controller
    puremvc_controller_removeController(storage, "ControllerTestKey7");

    // cleanup
    puremvc_controller_removeController(storage, "ControllerTestKey7");
    // puremvc_view_removeView("ControllerTestKey7");
}

void testRegisterAndRemoveMultipleCommands() {
    struct ControllerMap *storage[] = { &(struct ControllerMap) { // supply empty key storage
        .key = "",
        .controller = { .commandMap = (struct CommandMap *[]){} },
    }, NULL };

    struct IController *controller = puremvc_controller_getInstance(storage, "ControllerTestKey8");
    struct Controller *self = (struct Controller *) controller;
    struct View *view = (struct View *) self->view;

    struct INotification *notification0 = puremvc_notification_init(&(struct Notification){0}.base, "command0", NULL, NULL);
    controller->executeCommand(controller, notification0); // crash test
    controller->executeCommand(controller, notification0);

    // Register one command, verify associations and remove it
    controller->registerCommand(controller, "command0", puremvc_simple_command_init);
    assert(strcmp(self->commandMap[0]->key, "command0") == 0);
    assert(self->commandMap[0]->factory == puremvc_simple_command_init);
    assert(strcmp(view->observerMap[0]->key, "command0") == 0);
    assert(view->observerMap[0]->observers[0]->getContext(view->observerMap[0]->observers[0]) == controller);
    controller->removeCommand(controller, NULL, "command0");
    assert(self->commandMap[0]->key[0] == '\0');

    // Register four commands and verify that each is correctly associated to their dictionaries and observers
    controller->registerCommand(controller, "command1", puremvc_simple_command_init);
    assert(strcmp(self->commandMap[0]->key, "command1") == 0);
    assert(self->commandMap[0]->factory == puremvc_simple_command_init);
    assert(strcmp(view->observerMap[0]->key, "command1") == 0);
    assert(view->observerMap[0]->observers[0]->getContext(view->observerMap[0]->observers[0]) == controller);

    controller->registerCommand(controller, "command2", puremvc_simple_command_init);
    assert(strcmp(self->commandMap[1]->key, "command2") == 0);
    assert(self->commandMap[1]->factory == puremvc_simple_command_init);
    assert(strcmp(view->observerMap[1]->key, "command2") == 0);
    assert(view->observerMap[1]->observers[0]->getContext(view->observerMap[1]->observers[0]) == controller);

    controller->registerCommand(controller, "command3", puremvc_simple_command_init);
    assert(strcmp(self->commandMap[2]->key, "command3") == 0);
    assert(self->commandMap[2]->factory == puremvc_simple_command_init);
    assert(strcmp(view->observerMap[2]->key, "command3") == 0);
    assert(view->observerMap[2]->observers[0]->getContext(view->observerMap[2]->observers[0]) == controller);

    controller->registerCommand(controller, "command4", puremvc_simple_command_init);
    assert(strcmp(self->commandMap[3]->key, "command4") == 0);
    assert(self->commandMap[3]->factory == puremvc_simple_command_init);
    assert(strcmp(view->observerMap[3]->key, "command4") == 0);
    assert(view->observerMap[3]->observers[0]->getContext(view->observerMap[3]->observers[0]) == controller);

    // Remove the second command (middle) and verify that remaining commands and observers 3, 4 are shifted correctly
    controller->removeCommand(controller, NULL, "command2");
    assert(strcmp(self->commandMap[0]->key, "command1") == 0);
    assert(self->commandMap[0]->factory == puremvc_simple_command_init);
    assert(strcmp(view->observerMap[1]->key, "command3") == 0);
    assert(view->observerMap[1]->observers[0]->getContext(view->observerMap[1]->observers[0]) == controller);
    assert(strcmp(view->observerMap[2]->key, "command4") == 0);
    assert(view->observerMap[2]->observers[0]->getContext(view->observerMap[2]->observers[0]) == controller);

    // Remove the last command and verify the remaining command 1, 3 stay in place
    controller->removeCommand(controller, NULL, "command4");
    assert(strcmp(self->commandMap[0]->key, "command1") == 0);
    assert(self->commandMap[0]->factory == puremvc_simple_command_init);
    assert(strcmp(view->observerMap[1]->key, "command3") == 0);
    assert(view->observerMap[1]->observers[0]->getContext(view->observerMap[1]->observers[0]) == controller);

    // Remove the first command and verify that subsequent command 3 shift left
    controller->removeCommand(controller, NULL, "command1");
    assert(strcmp(self->commandMap[0]->key, "command3") == 0);
    assert(self->commandMap[0]->factory == puremvc_simple_command_init);

    // Remove all remaining mediators and confirm that the dictionary key is cleared
    controller->removeCommand(controller, NULL, "command3");
    assert(self->commandMap[0]->key[0] == '\0');

    puremvc_controller_removeController(storage, "ControllerTestKey8");
    // puremvc_view_removeView("ControllerTestKey8");
    controller = NULL;
}

void TestViewShiftLeft() {
    struct ControllerMap *storage[] = { &(struct ControllerMap) { // supply empty key storage
        .key = "",
        .controller = { .commandMap = (struct CommandMap *[]){} },
    }, NULL };

    puremvc_controller_getInstance(storage, "controller1");
    puremvc_controller_getInstance(storage, "controller2");
    puremvc_controller_getInstance(storage, "controller3");
    puremvc_controller_getInstance(storage, "controller4");

    puremvc_controller_removeController(storage, "controller2"); // remove middle
    puremvc_controller_removeController(storage, "controller4"); // remove last
    puremvc_controller_removeController(storage, "controller1"); // remove first
    puremvc_controller_removeController(storage, "controller3"); // remove remaining

    // puremvc_view_removeView(storage, "controller2"); // remove middle // todo
    // puremvc_view_removeView(storage, "controller4"); // remove last
    // puremvc_view_removeView(storage, "controller1"); // remove first
    // puremvc_view_removeView(storage, "controller3"); // remove remaining
}
