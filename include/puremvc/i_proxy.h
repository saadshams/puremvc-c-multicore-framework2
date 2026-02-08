/**
 * @file i_proxy.h
 * @ingroup PureMVC
 * @brief IProxy Interface
 *
 * @author Saad Shams <saad.shams@puremvc.org>
 * @copyright BSD 3-Clause License
 */
#pragma once

#include "i_notifier.h"

#include <stddef.h>

#define PROXY_NAME "Proxy"

/**
 * @struct IProxy
 * @brief Represents the state of a data processor.
 */
struct IProxy {
    const char* (*getName)(const struct IProxy* self);
    void* (*getData)(const struct IProxy* self);
    void (*setData)(struct IProxy* self, void* data);
    struct INotifier *(*getNotifier)(const struct IProxy *self);

    void (*onRegister)(struct IProxy* self);
    void (*onRemove)(struct IProxy* self);
};

size_t puremvc_proxy_size(const char *name);

struct IProxy *puremvc_proxy_init(void *buffer, const char *name, void *data);
