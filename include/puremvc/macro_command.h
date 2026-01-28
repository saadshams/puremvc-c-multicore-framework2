/**
* @file macro_command.h
* @brief MacroCommand Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "simple_command.h"
#include "constants.h"

struct MacroCommand {
    struct SimpleCommand simple_command;
    struct Notifier notifier;

    struct SimpleCommand (*subCommands[MAX_COMMANDS])();
    size_t count;

    void (*initializeMacroCommand)(struct MacroCommand *self);
    void (*addSubCommand)(struct MacroCommand *self, struct SimpleCommand(*factory)());
    void (*execute)(const struct MacroCommand *self, struct Notification *notification);
};

struct MacroCommand puremvc_macro_command();
