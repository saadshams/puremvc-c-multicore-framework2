/**
 * @file i_notifier.h
 * @ingroup PureMVC
 * @brief INotifier Interface
 *
 * @author Saad Shams <saad.shams@puremvc.org>
 * @copyright BSD 3-Clause License
 */
#pragma once

struct IFacade; // Forward-declare IFacade to avoid including it and break reference cycle

struct INotifier {
    struct IFacade *(*getFacade)(const struct INotifier *self);

    const char *(*getMultitonKey)(const struct INotifier *self);

    void (*initializeNotifier)(struct INotifier *self, const char *key);

    void (*sendNotification)(const struct INotifier *self, const char *notificationName, void *body, const char *type);
};
