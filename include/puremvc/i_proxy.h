/**
 * @file i_proxy.h
 * @ingroup PureMVC
 * @brief IProxy Interface
 *
 * @author Saad Shams <saad.shams@puremvc.org>
 * @copyright BSD 3-Clause License
 */
#pragma once

/**
 * @struct IProxy
 * @brief Represents the state of a data processor.
 */
struct IProxy {
    const char* (*getName)(const struct IProxy* self);
    void* (*getData)(const struct IProxy* self);
    void (*setData)(struct IProxy* self, void* data);

    void (*onRegister)(struct IProxy* self);
    void (*onRemove)(struct IProxy* self);
};
