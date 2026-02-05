/**
* @file simple_command.h
* @brief SimpleCommand Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "i_command.h"

struct SimpleCommand {
    struct ICommand base;
};

struct ICommand *puremvc_simple_command(struct SimpleCommand *command);

void puremvc_simple_command_deinit(struct SimpleCommand *command);
