#include "macro_command_test.h"
#include "macro_command_test_command.h"
#include "macro_command_test_vo.h"

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
    printf("\033[1;36m[SUITE] %s\033[0m\n", "MacroCommandTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    test("testMacroCommandExecute", testMacroCommandExecute);

    printf("\n\033[1;32m[DONE] All tests in suite finished.\033[0m\n");
    return 0;
}

void testMacroCommandExecute() {
    struct MacroCommandTestVO vo = { .input = 5, 0, 0, 0};

    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), "MacroCommandTest", &vo, NULL);

    const struct ICommand *command = macro_command_test_command(alloca(puremvc_macro_command_size()));

    command->getNotifier(command)->initializeNotifier(command->getNotifier(command), "MacroCommandTestkey1");

    command->execute(command, notification);

    if (vo.result1 != 10) abort();
    if (vo.result2 != 25) abort();
    if (vo.result3 != 125) abort();
}
