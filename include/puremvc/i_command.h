/**
 * @file i_command.h
 * @ingroup PureMVC
 * @brief ICommand Interface
 *
 * @author Saad Shams <saad.shams@puremvc.org>
 * @copyright BSD 3-Clause License
 */
#pragma once

#include "i_notification.h"
#include "i_notifier.h"

/**
 * @struct ICommand
 * @brief Represents a command in the PureMVC C framework.
 *
 * A Command encapsulates the logic to be executed in response
 * to a notification. It uses an INotifier to send further notifications.
 * @see INotification
 */
struct ICommand {
    struct INotifier *(*getNotifier)(const struct ICommand *self);

    void (*execute)(const struct ICommand *self, struct INotification *notification);
};
