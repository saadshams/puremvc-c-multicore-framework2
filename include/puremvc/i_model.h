/**
 * @file i_model.h
 * @ingroup PureMVC
 * @brief IModel Interface
 *
 * @author Saad Shams <saad.shams@puremvc.org>
 * @copyright BSD 3-Clause License
 */
#pragma once

#include "i_proxy.h"

#include <stdbool.h>

struct ModelMap {
    const char *key;
    struct IModel *model;
};

struct ProxyMap {
    const char *key;
    struct IProxy *proxy;
};

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
    void (*initializeModel)(struct IModel *self, struct ProxyMap **proxyMap);

    bool (*registerProxy)(struct IModel *self, struct IProxy *(*factory)(void *buffer, const char *name, void *data), const char *name, void *data);

    struct IProxy *(*retrieveProxy)(const struct IModel *self, const char *proxyName);

    bool (*hasProxy)(const struct IModel *self, const char *proxyName);

    bool (*removeProxy)(struct IModel *self, const char *proxyName, struct IProxy **out);
};

size_t puremvc_model_size();

struct IModel *puremvc_model_init(void *buffer, const char *key);

struct IModel *puremvc_model_getInstance(struct ModelMap **modelMap, const char *key);

bool puremvc_model_removeModel(const char *key, struct IModel **out);
