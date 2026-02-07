/**
 * @file i_model.h
 * @ingroup PureMVC
 * @brief IModel Interface
 *
 * @author Saad Shams <saad.shams@puremvc.org>
 * @copyright BSD 3-Clause License
 */
#pragma once

#include "proxy.h"

#include <stdbool.h>

/**
 * @struct IModel
 * @brief Interface for the PureMVC Model component.
 *
 * The Model is responsible for maintaining a collection of
 * Proxy instances. Each Proxy is registered and accessed
 * by a unique name.
 *
 * Implementations must ensure thread-safety if used in
 * concurrent environments.
 */
struct IModel {
    void (*initializeModel)(struct IModel *self);

    bool (*registerProxy)(struct IModel *self, struct IProxy *(*factory)(struct IProxy *proxy, const char *name, void *data), const char *name, void *data);

    struct IProxy *(*retrieveProxy)(const struct IModel *self, const char *proxyName);

    bool (*hasProxy)(const struct IModel *self, const char *proxyName);

    struct Proxy(*removeProxy)(struct IModel *self, const char *proxyName);
};
