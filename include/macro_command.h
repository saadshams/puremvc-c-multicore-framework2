#ifndef PUREMVC_MACRO_COMMAND_H
#define PUREMVC_MACRO_COMMAND_H

#include "simple_command.h"
#include "array.h"

struct MacroCommand {
    struct SimpleCommand simpleCommand;
    Array *subCommands;

    void (*initializeMacroCommand)(struct MacroCommand *self);
    void (*addSubCommand)(struct MacroCommand *self, struct SimpleCommand *(*factory)(void));
};

struct MacroCommand *puremvc_macro_command_init(struct MacroCommand *self);

struct MacroCommand *puremvc_macro_command_alloc();

struct MacroCommand *puremvc_macro_command_new();

void puremvc_macro_command_free(struct MacroCommand **self);

#endif //PUREMVC_MACRO_COMMAND_H
