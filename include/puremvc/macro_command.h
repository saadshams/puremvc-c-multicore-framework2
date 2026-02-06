/**
* @file macro_command.h
* @brief MacroCommand Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "simple_command.h"

struct ICommand *puremvc_macro_command_init(struct SimpleCommand *simple_command);
