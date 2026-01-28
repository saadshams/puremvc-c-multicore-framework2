#include "macro_command_test_command.h"

#include <stdio.h>

#include "macro_command_test_command2.h"
#include "macro_command_test_sub1_command.h"
#include "macro_command_test_sub2_command.h"

static void initializeMacroCommand(struct MacroCommand *self) {
    // self->addSubCommand(self, macro_command_test_sub1_command);
    // self->addSubCommand(self, macro_command_test_sub2_command);
    printf("parent initialize macro command\n");
    fflush(stdout);
    self->addSubCommand(self, (struct SimpleCommand(*)()) macro_command_test_command2);
    printf("parent end initialized macro command\n");
}

struct MacroCommand macro_command_test_command() {
    printf("macro command init\n");
    fflush(stdout);
    struct MacroCommand command = puremvc_macro_command();
    command.initializeMacroCommand = initializeMacroCommand;
    return command;
}
