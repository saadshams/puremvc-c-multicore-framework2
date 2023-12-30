#include "macro_command_test_command.h"
#include "macro_command_test_vo.h"
#include "notifier.h"
#include <assert.h>
#include <stdio.h>

void testMacroCommandExecute() {
    MacroCommandTestVO vo = {5, 0, 0};

    struct Notification *notification = puremvc_notification_new("MacroCommandTest", &vo, NULL);

    struct MacroCommand *macroCommand = macro_command_test_command_new();
    macroCommand->simpleCommand.notifier->initializeNotifier(macroCommand->simpleCommand.notifier, "key1");
    macroCommand->simpleCommand.execute(&macroCommand->simpleCommand, notification);

    assert(vo.result1 == 10);

    assert(vo.result2 == 25);

    puremvc_macro_command_free(&macroCommand);
}

void testRegisterAndExecuteCommand() {
    // Create the controller, register the ControllerTestCommand to handle 'ControllerTest' notes
    struct Controller *controller = puremvc_controller_getInstance("ControllerTestKey1", puremvc_controller_new);
    controller->registerCommand(controller, "MacroCommandTest", (struct SimpleCommand *(*)(void)) macro_command_test_command_new);

    struct View *view = puremvc_view_getInstance("ControllerTestKey1", puremvc_view_new);

    // Create a 'ControllerTest' note
    MacroCommandTestVO vo = {5, 0, 0};
    struct Notification *notification = puremvc_notification_new("MacroCommandTest", &vo, NULL);

    view->notifyObservers(view, notification);

    // test assertions
    assert(vo.result1 == 10);
    assert(vo.result2 == 25);

    controller->removeCommand(controller, "MacroCommandTest");
    puremvc_notification_free(&notification);
    puremvc_controller_removeController("ControllerTest1");
    puremvc_view_removeView("ControllerTest1");
    controller = NULL;
}

int main() {
    testMacroCommandExecute();
    testRegisterAndExecuteCommand();
    return 0;
}
