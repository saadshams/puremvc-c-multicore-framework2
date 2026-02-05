#include "puremvc/macro_command.h"
#include "puremvc/controller.h"
#include "puremvc/notification.h"
#include "puremvc/view.h"

#include "macro_command_test.h"
#include "macro_command_test_command.h"
#include "macro_command_test_vo.h"

#include <assert.h>

int main() {
    testMacroCommandExecute();
    // testRegisterAndExecuteCommand();
    return 0;
}

void testMacroCommandExecute() {
    struct MacroCommandTestVO vo = { .input = 5, 0, 0, 0};

    struct INotification *notification = puremvc_notification(&(struct Notification){0}, "MacroCommandTest", &vo, NULL);

    const struct ICommand *command = macro_command_test_command(&(struct SimpleCommand){0});

    command->getNotifier(command)->initializeNotifier(command->getNotifier(command), "MacroCommandTestkey1");

    command->execute(command, notification);

    assert(vo.result1 == 10);
    assert(vo.result2 == 25);
    assert(vo.result3 == 125);
}

// void testRegisterAndExecuteCommand() {
//     struct IController *controller = puremvc_controller_getInstance("ControllerTestKey1");
//
//     controller->registerCommand(controller, "MacroCommandTest", macro_command_test_command);
//
//     const struct IView *view = puremvc_view_getInstance("ControllerTestKey1");
//
//     struct MacroCommandTestVO vo = {.input = 5, 0, 0, 0};
//     struct Notification notification = puremvc_notification("MacroCommandTest", &vo, NULL);
//
//     view->notifyObservers(view, &notification.base);
//
//     // test assertions
//     assert(vo.result1 == 10);
//     assert(vo.result2 == 25);
//     assert(vo.result3 == 125);
//
//     controller->removeCommand(controller, "MacroCommandTest");
//     puremvc_controller_removeController("ControllerTest1");
//     puremvc_view_removeView("ControllerTest1");
//     controller = NULL;
// }
