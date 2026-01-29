#include <assert.h>

#include "puremvc/macro_command.h"

#include "macro_command_test.h"
#include "macro_command_test_command.h"
#include "macro_command_test_vo.h"

int main() {
    testMacroCommandExecute();
    // testRegisterAndExecuteCommand();
    return 0;
}

void testMacroCommandExecute() {
    struct MacroCommandTestVO vo = { .input = 5, 0, 0, 0};

    struct Notification notification = puremvc_notification("MacroCommandTest", &vo, NULL);

    struct SimpleCommand command = macro_command_test_command();

    command.notifier.initializeNotifier(&command.notifier, "MacroCommandTestkey1");

    command.execute(&command, &notification);

    assert(vo.result1 == 10);
    assert(vo.result2 == 25);
    assert(vo.result3 == 125);
}

// void testRegisterAndExecuteCommand() {
//     const char *error = NULL;
//     struct IController *controller = puremvc_controller_getInstance("ControllerTestKey1", puremvc_controller_new, &error);
//     controller->initializeController(controller, &error);
//     assert(error == NULL);
//
//     controller->registerCommand(controller, "MacroCommandTest", (struct ICommand *(*)(const char **)) macro_command_test_comman, &error);
//     assert(error == NULL);
//
//     const struct IView *view = puremvc_view_getInstance("ControllerTestKey1", puremvc_view_new, &error);
//     assert(error == NULL);
//
//     struct MacroCommandTestVO *vo = malloc(sizeof(struct MacroCommandTestVO));
//     vo->input = 5;
//     struct Notification notification = puremvc_notification("MacroCommandTest", vo, NULL, &error);
//     assert(error == NULL);
//
//     view->notifyObservers(view, notification, &error);
//
//     // test assertions
//     assert(vo->result1 == 10);
//     assert(vo->result2 == 25);
//     assert(vo->result3 == 125);
//
//     controller->removeCommand(controller, "MacroCommandTest");
//     puremvc_notification_free(&notification);
//     puremvc_controller_removeController("ControllerTest1");
//     puremvc_view_removeView("ControllerTest1");
//     controller = NULL;
// }
