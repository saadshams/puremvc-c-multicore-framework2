#include <assert.h>
#include <string.h>

#include "model_test.h"
#include "puremvc/model.h"

#include "model_test_proxy.h"

int main() {
    testGetInstance();
    testRegisterAndRetrieveProxy();
    testRegisterAndRemoveProxy();
    testHasProxy();
    testOnRegisterAndOnRemove();
    testRemoveModel();
    testMultipleModels();
    testRegisterAndReplaceProxy();
    testRegisterAndRemoveMultipleProxies();
    TestModelShiftLeft();
    return 0;
}

void testGetInstance() {
    // Test Factory Method
    struct ModelMap *storage[] = { &(struct ModelMap) { // supply empty key storage
        .key = "",
        .model = {
            .multitonKey = "",
            .proxyMap = (struct ProxyMap*[]) { NULL },
        }
    }, NULL };

    const struct IModel *model = puremvc_model_getInstance(storage, "ModelTestKey1");

    assert(model != NULL);
    assert(strcmp(storage[0]->key, "ModelTestKey1") == 0);
    assert(&storage[0]->model != NULL);

    // test assertions
    assert(model == puremvc_model_getInstance(storage, "ModelTestKey1"));
    assert(puremvc_model_getInstance(NULL, "ModelTestKey1") == NULL);
    puremvc_model_removeModel(storage, "ModelTestKey1");

    assert(storage[0]->key[0] == '\0');
    assert(&storage[0]->model != NULL);
    model = NULL;
}

void testRegisterAndRetrieveProxy() {
    // register a new and retrieve it.
    struct ModelMap *modelMap[] = { &(struct ModelMap) { // supply empty key storage with one slot for proxy
        .model = { .proxyMap = (struct ProxyMap*[]) { &(struct ProxyMap){0}, NULL } }
    }, NULL };

    struct IModel *model = puremvc_model_getInstance(modelMap, "ModelTestKey2");

    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    model->registerProxy(model, puremvc_proxy_init("colors", colors));

    const struct IProxy *proxy = model->retrieveProxy(model, "colors");
    assert(proxy != NULL);
    const char **data = proxy->getData(proxy);

    // test assertions
    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);

    const struct Proxy removedProxy = model->removeProxy(model, "colors");
    assert(strcmp(removedProxy.base.getName(&removedProxy.base), "colors") == 0);

    assert(model->retrieveProxy(model, "colors") == NULL);
    puremvc_model_removeModel(modelMap, "ModelTestKey2");
    model = NULL;
}

void testRegisterAndRemoveProxy() {
    // register a new, remove it, then try to retrieve it
    struct ModelMap *modelMap[] = { &(struct ModelMap) {
        .model = { .proxyMap = (struct ProxyMap*[]) { &(struct ProxyMap){0}, NULL } }
    }, NULL };

    struct IModel *model = puremvc_model_getInstance(modelMap, "ModelTestKey3");

    int *sizes = (int []) {1, 2, 3, 0};
    struct Proxy p = puremvc_proxy_init("sizes", sizes);
    model->registerProxy(model, p);

    // remove the new
    const struct Proxy removedProxy = model->removeProxy(model, "sizes");

    // assert that we removed the appropriate new
    assert(strcmp(removedProxy.name, "sizes") == 0);

    // ensure that the new is no longer retrievable from the model
    assert(model->retrieveProxy(model, "sizes") == NULL);

    puremvc_model_removeModel(modelMap, "ModelTestKey3");
    model = NULL;
}

void testHasProxy() {
    // register a new
    struct ModelMap *modelMap[] = { &(struct ModelMap) {
        .model = { .proxyMap = (struct ProxyMap*[]) { &(struct ProxyMap){0}, NULL } }
    }, NULL };

    struct IModel *model = puremvc_model_getInstance(modelMap, "ModelTestKey4");

    const char **aces = (const char *[]) {"clubs", "spades", "hearts", "diamonds", NULL};
    struct Proxy p = puremvc_proxy_init("aces", aces);
    model->registerProxy(model, p);

    // assert that the model.hasProxy method returns true
    // for that new name
    assert(model->hasProxy(model, "aces") == true);

    // remove the new
    const struct Proxy proxy = model->removeProxy(model, "aces");
    assert(strcmp(proxy.name, "aces") == 0);

    // assert that the model.hasProxy method returns false
    // for that new name
    assert(model->hasProxy(model, "aces") == false);

    puremvc_model_removeModel(modelMap, "ModelTestKey4");
    model = NULL;
}

void testOnRegisterAndOnRemove() {
    // Get a Multiton Model instance
    struct ModelMap *modelMap[] = { &(struct ModelMap) {
        .model = { .proxyMap = (struct ProxyMap*[]) { &(struct ProxyMap){0}, NULL } }
    }, NULL };

    struct IModel *model = puremvc_model_getInstance(modelMap, "ModelTestKey5");

    // Create and register the test proxy
    model->registerProxy(model, model_test_proxy("ModelTestProxy", NULL));

    // assert that onRegister was called, and the new responded by setting its data accordingly
    const struct IProxy *proxy = model->retrieveProxy(model, "ModelTestProxy");
    assert(strcmp(proxy->getData(proxy), ON_REGISTER_CALLED) == 0);

    // Remove the proxy
    const struct Proxy removedProxy = model->removeProxy(model, "ModelTestProxy");
    assert(strcmp(removedProxy.name, "ModelTestProxy") == 0);

    // assert that onRemove was called, and the new responded by setting its data accordingly
    assert(strcmp(removedProxy.data, ON_REMOVE_CALLED) == 0);

    puremvc_model_removeModel(modelMap, "ModelTestKey5");
    model = NULL;
}

void testRemoveModel() {
    // Get a Multiton Model instance
    struct ModelMap *modelMap[] = { &(struct ModelMap) {
        .model = { .proxyMap = (struct ProxyMap*[]) { &(struct ProxyMap){0}, NULL } }
    }, NULL };

    puremvc_model_getInstance(modelMap, "ModelTestKey6");

    // remove the model
    puremvc_model_removeModel(modelMap, "ModelTestKey6");

    // try removing again
    puremvc_model_removeModel(modelMap, "ModelTestKey6");
}

void testMultipleModels() {
    // Get a Multiton Model instance
    struct ModelMap *modelMap[] = {  // provide two empty key model slots, each one with one slot for proxy
        &(struct ModelMap) {
            .model = { .proxyMap = (struct ProxyMap*[]) { &(struct ProxyMap){0}, NULL } }
        },
        &(struct ModelMap) {
            .model = { .proxyMap = (struct ProxyMap*[]) { &(struct ProxyMap){0}, NULL } }
        },
        NULL
    };

    struct IModel *model1 = puremvc_model_getInstance(modelMap, "ModelTestKey7");
    struct IModel *model2 = puremvc_model_getInstance(modelMap, "ModelTestKey8");

    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    const char **aces = (const char *[]) { "clubs", "spades", "hearts", "diamonds", NULL};

    struct Proxy p1 = puremvc_proxy_init("colors", colors);
    model1->registerProxy(model1, p1);
    struct Proxy p2 = puremvc_proxy_init("aces", aces);
    model2->registerProxy(model2, p2);

    assert(model1->hasProxy(model1, "colors"));
    assert(!model2->hasProxy(model2, "colors"));

    assert(!model1->hasProxy(model1, "aces"));
    assert(model2->hasProxy(model2, "aces"));

    const struct Proxy proxy = model1->removeProxy(model1, "colors");
    assert(strcmp(proxy.name, "colors") == 0);
    const struct Proxy removedProxy = model2->removeProxy(model2, "aces");
    assert(strcmp(removedProxy.name, "aces") == 0);

    puremvc_model_removeModel(modelMap, "ModelTestKey7");
    puremvc_model_removeModel(modelMap, "ModelTestKey8");
}

void testRegisterAndReplaceProxy() {
    struct ModelMap *modelMap[] = { &(struct ModelMap) { // empty model key storage with two slots for proxy
        .model = {
            .proxyMap = (struct ProxyMap*[]) { &(struct ProxyMap){0}, &(struct ProxyMap){0}, NULL }
        }
    }, NULL };

    struct IModel *model = puremvc_model_getInstance(modelMap, "ModelTestKey9");

    int *sizes = (int []) {1, 0};
    struct Proxy p1 = puremvc_proxy_init("sizes", sizes);
    model->registerProxy(model, p1);

    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    struct Proxy p2 = puremvc_proxy_init("sizes", colors);
    model->registerProxy(model, p2);

    const struct IProxy *proxy = model->retrieveProxy(model, "sizes");

    assert(proxy != NULL);
    const char **data = proxy->getData(proxy);

    // test assertions
    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);

    const struct Proxy removedProxy = model->removeProxy(model, "sizes");
    assert(strcmp(removedProxy.name, "sizes") == 0);

    assert(model->retrieveProxy(model, "sizes") == NULL);
    puremvc_model_removeModel(modelMap, "ModelTestKey9");
    model = NULL;
}

void testRegisterAndRemoveMultipleProxies() {
    struct ModelMap *modelMap[] = { &(struct ModelMap) { // empty key model storage with four slots for proxy
        .model = {
            .proxyMap = (struct ProxyMap*[]) {
                &(struct ProxyMap){0}, &(struct ProxyMap){0}, &(struct ProxyMap){0}, &(struct ProxyMap){0}, NULL
            }
        }
    }, NULL };

    struct IModel *model = puremvc_model_getInstance(modelMap, "ModelTestKey10");
    const struct Model *self = (struct Model *) model;

    // Register four proxies and verify that each is correctly associated to their dictionaries
    model->registerProxy(model, puremvc_proxy_init("proxy1", NULL));
    assert(strcmp(self->proxyMap[0]->key, "proxy1") == 0);
    assert(strcmp(self->proxyMap[0]->proxy.name, "proxy1") == 0);

    model->registerProxy(model, puremvc_proxy_init("proxy2", NULL));
    assert(strcmp(self->proxyMap[1]->key, "proxy2") == 0);
    assert(strcmp(self->proxyMap[1]->proxy.name, "proxy2") == 0);

    model->registerProxy(model, puremvc_proxy_init("proxy3", NULL));
    assert(strcmp(self->proxyMap[2]->key, "proxy3") == 0);
    assert(strcmp(self->proxyMap[2]->proxy.name, "proxy3") == 0);

    model->registerProxy(model, puremvc_proxy_init("proxy4", NULL));
    assert(strcmp(self->proxyMap[3]->key, "proxy4") == 0);
    assert(strcmp(self->proxyMap[3]->proxy.name, "proxy4") == 0);

    // Remove the second proxy (middle) and verify that remaining mediators 3, 4 are shifted correctly
    model->removeProxy(model, "proxy2");
    assert(strcmp(self->proxyMap[0]->key, "proxy1") == 0);
    assert(strcmp(self->proxyMap[0]->proxy.name, "proxy1") == 0);
    assert(strcmp(self->proxyMap[1]->key, "proxy3") == 0);
    assert(strcmp(self->proxyMap[1]->proxy.name, "proxy3") == 0);
    assert(strcmp(self->proxyMap[2]->key, "proxy4") == 0);
    assert(strcmp(self->proxyMap[2]->proxy.name, "proxy4") == 0);

    // Remove the last proxy and verify the remaining mediator3 stay in place
    model->removeProxy(model, "proxy4");
    assert(strcmp(self->proxyMap[0]->key, "proxy1") == 0);
    assert(strcmp(self->proxyMap[0]->proxy.name, "proxy1") == 0);
    assert(strcmp(self->proxyMap[1]->key, "proxy3") == 0);
    assert(strcmp(self->proxyMap[1]->proxy.name, "proxy3") == 0);

    // Remove the first mediator and verify that subsequent mediator 3 shift left
    model->removeProxy(model, "proxy1");
    assert(strcmp(self->proxyMap[0]->key, "proxy3") == 0);
    assert(strcmp(self->proxyMap[0]->proxy.name, "proxy3") == 0);

    // Remove the remaining mediator3 and confirm that the dictionary is NULL
    model->removeProxy(model, "proxy3");
    assert(self->proxyMap[0]->key[0] == '\0'); // proxyMap is empty
    assert(self->proxyMap[1]->key[0] == '\0');
    assert(self->proxyMap[2]->key[0] == '\0');
    assert(self->proxyMap[3]->key[0] == '\0');

    puremvc_model_removeModel(modelMap, "ModelTestKey10");
    model = NULL;
}

void TestModelShiftLeft() {
    struct ModelMap *modelMap[] = { // empty key model storage with 4 slots
        &(struct ModelMap) { .model = { .proxyMap = (struct ProxyMap*[]) { NULL } } },
        &(struct ModelMap) { .model = { .proxyMap = (struct ProxyMap*[]) { NULL } } },
        &(struct ModelMap) { .model = { .proxyMap = (struct ProxyMap*[]) { NULL } } },
        &(struct ModelMap) { .model = { .proxyMap = (struct ProxyMap*[]) { NULL } } },
        NULL
    };

    puremvc_model_getInstance(modelMap, "model0");
    assert(strcmp(modelMap[0]->key, "model0") == 0);
    puremvc_model_getInstance(modelMap, "model1");
    assert(strcmp(modelMap[1]->key, "model1") == 0);
    puremvc_model_getInstance(modelMap, "model2");
    assert(strcmp(modelMap[2]->key, "model2") == 0);
    puremvc_model_getInstance(modelMap, "model3");
    assert(strcmp(modelMap[3]->key, "model3") == 0);

    puremvc_model_removeModel(modelMap, "model2"); // remove middle, remaining 0, 1, 3
    assert(strcmp(modelMap[0]->key, "model0") == 0);
    assert(strcmp(modelMap[1]->key, "model1") == 0);
    assert(strcmp(modelMap[2]->key, "model3") == 0);
    assert(modelMap[3]->key[0] == '\0');

    puremvc_model_removeModel(modelMap, "model3"); // remove last, remaining 0, 1
    assert(strcmp(modelMap[0]->key, "model0") == 0);
    assert(strcmp(modelMap[1]->key, "model1") == 0);
    assert(modelMap[2]->key[0] == '\0');
    assert(modelMap[3]->key[0] == '\0');

    puremvc_model_removeModel(modelMap, "model0"); // remove first, remaining 1
    assert(strcmp(modelMap[0]->key, "model1") == 0);
    assert(modelMap[1]->key[0] == '\0');
    assert(modelMap[2]->key[0] == '\0');
    assert(modelMap[3]->key[0] == '\0');

    puremvc_model_removeModel(modelMap, "model1"); // remove remaining
    assert(modelMap[0]->key[0] == '\0');
    assert(modelMap[1]->key[0] == '\0');
    assert(modelMap[2]->key[0] == '\0');
    assert(modelMap[3]->key[0] == '\0');
}
