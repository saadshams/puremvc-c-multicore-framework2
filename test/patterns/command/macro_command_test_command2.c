#include "macro_command_test_command2.h"

#include <stdio.h>

#include "macro_command_test_sub3_command.h"

static void initializeMacroCommand(struct MacroCommand *self) {
    self->addSubCommand(self, macro_command_test_sub3_command);
}

struct MacroCommand macro_command_test_command2() {
    printf("macro command 2 init\n");
    fflush(stdout);
    struct MacroCommand command = puremvc_macro_command();
    command.initializeMacroCommand = initializeMacroCommand;
    printf("end macro command 2 init\n");
    return command;
}
