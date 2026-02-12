#include "facade_test.h"
#include "facade_test_command.h"
#include "facade_test_vo.h"

#include "puremvc/i_facade.h"
#include "puremvc/i_observer.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void test(const char *name, void (*callback)(void)) {
    printf("\033[0;34m[RUNNING]\033[0m %s...\n", name);
    fflush(stdout);

    callback();

    printf("\033[0;32m[PASSED]\033[0m %s\n", name);
    fflush(stdout);
}

void abc(){}

int main() {
    printf("\n\033[1;36m================================================\033[0m\n");
    printf("\033[1;36m[SUITE] %s\033[0m\n", "FacadeTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    test("testGetInstance", testGetInstance);
    test("testRegisterCommandAndSendNotification", testRegisterCommandAndSendNotification);
    test("testRegisterAndRemoveCommandAndSendNotification", testRegisterAndRemoveCommandAndSendNotification);
    test("testRegisterAndRetrieveProxy", testRegisterAndRetrieveProxy); //
    test("testRegisterAndRemoveProxy", testRegisterAndRemoveProxy); //
    test("testRegisterRetrieveAndRemoveMediator", testRegisterRetrieveAndRemoveMediator);
    test("testHasProxy", testHasProxy);
    test("testHasMediator", testHasMediator);
    test("testHasCommand", testHasCommand);
    test("testHasCoreAndRemoveCore", testHasCoreAndRemoveCore);
    test("testFacadeMapShiftLeft", testFacadeMapShiftLeft);

    printf("\n\033[1;32m[DONE] All tests in suite finished.\033[0m\n");
    return 0;
}

void testGetInstance() {
    struct FacadeMap **instanceMap = (struct FacadeMap *[]) {
        &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) },
        NULL
    };

    // Test Factory Method
    struct IFacade *facade = puremvc_facade_getInstance(instanceMap, "FacadeTestKey1");

    // test assertions
    assert(facade != NULL);
    assert(facade == puremvc_facade_getInstance(instanceMap, "FacadeTestKey1"));

    struct IFacade *removedFacade = NULL;
    assert(puremvc_facade_removeFacade("FacadeTestKey1", &removedFacade) == true);
    assert(instanceMap[0]->key == NULL);
}

void testRegisterCommandAndSendNotification() {
    struct ViewMap **viewMap = (struct ViewMap *[]) {
        &(struct ViewMap){ .view = alloca(puremvc_view_size()) },
        NULL
    };
    struct ObserverMap **observerMap = (struct ObserverMap *[]) { &(struct ObserverMap) {
        .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } }, NULL
    };
    struct IView *view = puremvc_view_getInstance(viewMap, "FacadeTestKey2");
    view->initializeView(view, observerMap, NULL);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) {
        &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) },
        NULL
    };
    struct CommandMap **commandMap = (struct CommandMap *[]) { &(struct CommandMap){}, NULL };
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "FacadeTestKey2");
    controller->initializeController(controller, view, commandMap);

    struct ModelMap **modelMap = (struct ModelMap *[]) {
        &(struct ModelMap){ .model = alloca(puremvc_model_size()) },
        NULL
    };
    struct IModel *model = puremvc_model_getInstance(modelMap, "FacadeTestKey2");
    model->initializeModel(model, NULL);

    struct FacadeMap **facadeMap = (struct FacadeMap *[]) {
        &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) },
        NULL
    };

    // Create the Facade, register the FacadeTestCommand to
    // handle 'FacadeTest' notifications
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey2");
    facade->initializeFacade(facade, model, view, controller);

    assert(facade->registerCommand(facade, "FacadeTestNote", test_facade_command) == true);

    // Send notification. The Command associated with the event
    // (FacadeTestCommand) will be invoked, and will multiply
    // the vo.input value by 2 and set the result on vo.result
    struct FacadeTestVO vo = {32};
    facade->sendNotification(facade, "FacadeTestNote", &vo, NULL);

    // test assertions
    assert(vo.result == 64);

    struct ICommand *(*factory)(void *) = 0;
    assert(facade->removeCommand(facade, "FacadeTestNote", &factory) == true);

    struct IFacade *removedFacade = NULL;
    assert(puremvc_facade_removeFacade("FacadeTestKey2", &removedFacade) == true);
}

void testRegisterAndRemoveCommandAndSendNotification() {
    struct ViewMap **viewMap = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct ObserverMap **observerMap = (struct ObserverMap *[]) {
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        NULL
    };
    struct IView *view = puremvc_view_getInstance(viewMap, "FacadeTestKey3");
    view->initializeView(view, observerMap, NULL);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) { &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) }, NULL };
    struct CommandMap **commandMap = (struct CommandMap *[]) { &(struct CommandMap){}, NULL };
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "FacadeTestKey3");
    controller->initializeController(controller, view, commandMap);

    // Create the Facade, register the FacadeTestCommand to
    // handle 'FacadeTest' events
    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL };
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey3");
    facade->initializeFacade(facade, NULL, view, controller);

    assert(facade->registerCommand(facade, "FacadeTestNote", test_facade_command) == true);
    assert(facade->removeCommand(facade, "FacadeTestNote", NULL) == true);

    // Send notification. The Command associated with the event
    // (FacadeTestCommand) will NOT be invoked, and will NOT multiply
    // the vo.input value by 2
    struct FacadeTestVO vo = {32};
    facade->sendNotification(facade, "FacadeTestNote", &vo, NULL);

    // test assertions
    assert(vo.result == 0);

    struct IFacade *removedFacade = NULL;
    assert(puremvc_facade_removeFacade("FacadeTestKey3", &removedFacade) == true);
}

void testRegisterAndRetrieveProxy() {
    struct ModelMap **modelMap = (struct ModelMap *[]) { &(struct ModelMap){ .model = alloca(puremvc_model_size()) }, NULL };
    struct ProxyMap **proxyMap = (struct ProxyMap *[]) { &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) }, NULL };
    struct IModel *model = puremvc_model_getInstance(modelMap, "FacadeTestKey4");
    model->initializeModel(model, proxyMap);

    // register a facade and retrieve it.
    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL};
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey4");
    facade->initializeFacade(facade, model, NULL, NULL);

    static char *colors[] = { "red", "green", "blue", NULL};

    assert(facade->registerProxy(facade, puremvc_proxy_init, "colors", colors) == true);
    const struct IProxy *proxy = facade->retrieveProxy(facade, "colors");

    // test assertions
    assert(proxy != NULL);

    // retrieve data from proxy
    // const char **data = proxy->getData(proxy);

    // test assertions
    // assert(data != NULL);
    // assert(strcmp(data[0], "red") == 0);
    // assert(strcmp(data[1], "green") == 0);
    // assert(strcmp(data[2], "blue") == 0);

    // struct IProxy *removedProxy = NULL;
    // assert(facade->removeProxy(facade, "colors", &removedProxy) == true);

    // struct IFacade *removedFacade = NULL;
    // assert(puremvc_facade_removeFacade("FacadeTestKey4", &removedFacade) == true);
}

void testRegisterAndRemoveProxy() {
    // struct ModelMap **modelMap = (struct ModelMap *[]) { &(struct ModelMap){ .model = alloca(puremvc_model_size()) }, NULL };
    // struct ProxyMap **proxyMap = (struct ProxyMap *[]) { &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) }, NULL };
    // struct IModel *model = puremvc_model_getInstance(modelMap, "FacadeTestKey5");
    // model->initializeModel(model, proxyMap);
    //
    // // register a proxy, remove it, then try to retrieve it
    // struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL};
    // struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey5");
    // facade->initializeFacade(facade, model, NULL, NULL);
    //
    // static int sizes[] = { 7, 13, 21, 0 }; // 0 is the sentinel
    // assert(facade->registerProxy(facade, puremvc_proxy_init, "sizes", sizes) == true);;
    //
    // const struct IProxy *proxy = facade->retrieveProxy(facade, "sizes");
    //
    // // 1. Retrieve as the raw pointer (void * becomes int *)
    // int *data = proxy->getData(proxy);
    //
    // // 2. Test assertions
    // assert(data != NULL);
    // assert(data[0] == 7);
    // assert(data[1] == 13);
    // assert(data[2] == 21);
    // assert(data[3] == 0); // Checking the sentinel
    //
    // // remove the proxy
    // struct IProxy *removedProxy = NULL;
    // assert(facade->removeProxy(facade, "sizes", &removedProxy) == true);
    //
    // // assert that we removed the appropriate proxy
    // assert(strcmp(removedProxy->getName(removedProxy), "sizes") == 0);
    //
    // // test assertions - make sure we can no longer retrieve the proxy from the model
    // assert(facade->retrieveProxy(facade, "sizes") == NULL);
    //
    // struct IFacade *removedFacade = NULL;
    // assert(puremvc_facade_removeFacade("FacadeTestKey5", &removedFacade) == true);
}

void testRegisterRetrieveAndRemoveMediator() {
    struct ViewMap **viewMap = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct MediatorMap **mediatorMap = (struct MediatorMap *[]){ &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) }, NULL };
    struct IView *view = puremvc_view_getInstance(viewMap, "FacadeTestKey6");
    view->initializeView(view, NULL, mediatorMap);

    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL};
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey6");
    facade->initializeFacade(facade, NULL, view, NULL);

    // register a mediator, remove it, then try to retrieve it
    struct Object {int x;} object;
    assert(facade->registerMediator(facade, puremvc_mediator_init, MEDIATOR_NAME, &object) == true);

    // retrieve the mediator
    assert(facade->retrieveMediator(facade, MEDIATOR_NAME) != NULL);

    // remove the mediator
    struct IMediator *removedMediator = NULL;
    assert(facade->removeMediator(facade, MEDIATOR_NAME, &removedMediator) == true);

    // assert that we have removed the appropriate mediator
    assert(strcmp(removedMediator->getName(removedMediator), MEDIATOR_NAME) == 0);

    // assert that the mediator is no longer retrievable
    assert(facade->retrieveMediator(facade, MEDIATOR_NAME) == NULL);

    struct IFacade *removedFacade = NULL;
    assert(puremvc_facade_removeFacade("FacadeTestKey6", &removedFacade) == true);
}

void testHasProxy() {
    struct ModelMap **modelMap = (struct ModelMap *[]) { &(struct ModelMap){ .model = alloca(puremvc_model_size()) }, NULL };
    struct ProxyMap **proxyMap = (struct ProxyMap *[]) { &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) }, NULL };
    struct IModel *model = puremvc_model_getInstance(modelMap, "FacadeTestKey7");
    model->initializeModel(model, proxyMap);

    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL};
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey7");
    facade->initializeFacade(facade, model, NULL, NULL);

    static int sizes[] = { 7, 13, 21, 0 }; // 0 is the sentinel

    // register a Proxy
    assert(facade->registerProxy(facade, puremvc_proxy_init, "hasProxyTest", sizes) == true);

    // assert that the model.hasProxy method returns true
    // for that new name
    assert(facade->hasProxy(facade, "hasProxyTest") == true);

    assert(facade->removeProxy(facade, "hasProxyTest", NULL) == true);;
    assert(facade->hasProxy(facade, "hasProxyTest") == false);

    assert(puremvc_facade_removeFacade("FacadeTestKey7", NULL) == true);
}

void testHasMediator() {
    struct ViewMap **viewMap = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct MediatorMap **mediatorMap = (struct MediatorMap *[]){ &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) }, NULL };
    struct IView *view = puremvc_view_getInstance(viewMap, "FacadeTestKey8");
    view->initializeView(view, NULL, mediatorMap);

    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL};
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey8");
    facade->initializeFacade(facade, NULL, view, NULL);

    // register a Mediator
    struct Object {int x;} object;
    assert(facade->registerMediator(facade, puremvc_mediator_init, "facadeHasMediatorTest", &object) == true);

    // assert that the facade.hasMediator method returns true
    // for that mediator name
    assert(facade->hasMediator(facade, "facadeHasMediatorTest") == true);

    assert(facade->removeMediator(facade, "facadeHasMediatorTest", NULL) == true);

    // assert that the facade.hasMediator method returns false
    // for that mediator name
    assert(facade->hasMediator(facade, "facadeHasMediatorTest") == false);
    assert(puremvc_facade_removeFacade("FacadeTestKey8", NULL) == true);
}

void testHasCommand() {
    struct ViewMap **viewMap = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct ObserverMap **observerMap = (struct ObserverMap *[]) {
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        NULL
    };
    struct IView *view = puremvc_view_getInstance(viewMap, "FacadeTestKey9");
    view->initializeView(view, observerMap, NULL);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) { &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) }, NULL };
    struct CommandMap **commandMap = (struct CommandMap *[]) { &(struct CommandMap){}, NULL };
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "FacadeTestKey10");
    controller->initializeController(controller, view, commandMap);

    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL };
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey10");
    facade->initializeFacade(facade, NULL, view, controller);

    // register the ControllerTestCommand to handle 'hasCommandTest' notes
    assert(facade->registerCommand(facade, "facadeHasCommandTest", test_facade_command) == true);

    // test that hasCommand returns true for hasCommandTest notifications
    assert(facade->hasCommand(facade, "facadeHasCommandTest") == true);

    // Remove the Command from the Controller
    struct ICommand *(*removedCommand)(void *) = 0;
    assert(facade->removeCommand(facade, "facadeHasCommandTest", &removedCommand) == true);
    assert(removedCommand == test_facade_command);

    // test that hasCommand returns false for hasCommandTest notifications
    assert(facade->hasCommand(facade, "facadeHasCommandTest") == false);

    assert(puremvc_facade_removeFacade("FacadeTestKey10", NULL) == true);
}

void testHasCoreAndRemoveCore() {
    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL };
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey10");
    facade->initializeFacade(facade, NULL, NULL, NULL);

    // assert that the Facade.hasCore method returns false first
    assert(puremvc_facade_hasCore("unregistered") == false);

    // register a Core
    puremvc_facade_getInstance(facadeMap, "FacadeTestKey10");

    assert(puremvc_facade_hasCore("FacadeTestKey10") == true);

    // remove the Core
    puremvc_facade_removeFacade("FacadeTestKey10", NULL);

    // assert that the Facade.hasCore method returns false now that the core has been removed.
    assert(puremvc_facade_hasCore("FacadeTestKey10") == false);
}

void testFacadeMapShiftLeft() {
    struct FacadeMap **instanceMap = (struct FacadeMap *[]) { // facadeMap with 4 slots for the instance
        &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) },
        &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) },
        &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) },
        &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) },
        NULL
    };

    assert(puremvc_facade_getInstance(instanceMap, "facade0") != NULL);
    assert(strcmp(instanceMap[0]->key, "facade0") == 0);
    const char **key0 = (const char **)((char *) instanceMap[0]->facade + sizeof(struct IFacade));
    assert(strcmp(*key0, "facade0") == 0);
    assert(puremvc_facade_hasCore("facade0") == true);

    assert(puremvc_facade_getInstance(instanceMap, "facade1") != NULL);
    assert(strcmp(instanceMap[1]->key, "facade1") == 0);
    const char **key1 = (const char **)((char *) instanceMap[1]->facade + sizeof(struct IFacade));
    assert(strcmp(*key1, "facade1") == 0);
    assert(puremvc_facade_hasCore("facade1") == true);

    assert(puremvc_facade_getInstance(instanceMap, "facade2") != NULL);
    assert(strcmp(instanceMap[2]->key, "facade2") == 0);
    const char **key2 = (const char **)((char *) instanceMap[2]->facade + sizeof(struct IFacade));
    assert(strcmp(*key2, "facade2") == 0);
    assert(puremvc_facade_hasCore("facade2") == true);

    assert(puremvc_facade_getInstance(instanceMap, "facade3") != NULL);
    assert(strcmp(instanceMap[3]->key, "facade3") == 0);
    const char **key3 = (const char **)((char *) instanceMap[3]->facade + sizeof(struct IFacade));
    assert(strcmp(*key3, "facade3") == 0);
    assert(puremvc_facade_hasCore("facade3") == true);

    // remove
    struct IFacade *facade1 = NULL; // remove middle1, remaining 0, 2, 3
    assert(puremvc_facade_removeFacade("facade1", &facade1) == true);
    assert(strcmp(instanceMap[0]->key, "facade0") == 0);
    assert(strcmp(instanceMap[1]->key, "facade2") == 0);
    assert(strcmp(instanceMap[2]->key, "facade3") == 0);
    assert(instanceMap[3]->key == NULL);
    assert(instanceMap[4] == NULL);
    assert(puremvc_facade_hasCore("facade1") == false);

    struct IFacade *facade3 = NULL; // remove last3, remaining 0, 2
    assert(puremvc_facade_removeFacade("facade3", &facade3) == true);
    assert(strcmp(instanceMap[0]->key, "facade0") == 0);
    assert(strcmp(instanceMap[1]->key, "facade2") == 0);
    assert(instanceMap[2]->key == NULL);
    assert(instanceMap[3]->key == NULL);
    assert(instanceMap[4] == NULL);
    assert(puremvc_facade_hasCore("facade3") == false);

    struct IFacade *facade0 = NULL; // remove first, remaining 2
    assert(puremvc_facade_removeFacade("facade0", &facade0) == true);
    assert(strcmp(instanceMap[0]->key, "facade2") == 0);
    assert(instanceMap[1]->key == NULL);
    assert(instanceMap[2]->key == NULL);
    assert(instanceMap[3]->key == NULL);
    assert(instanceMap[4] == NULL);
    assert(puremvc_facade_hasCore("facade0") == false);

    struct IFacade *facade2 = NULL; // remove remaining
    assert(puremvc_facade_removeFacade("facade2", &facade2) == true);
    assert(instanceMap[0]->key == NULL);
    assert(instanceMap[1]->key == NULL);
    assert(instanceMap[2]->key == NULL);
    assert(instanceMap[3]->key == NULL);
    assert(instanceMap[4] == NULL);
    assert(puremvc_facade_hasCore("facade2") == false);
}
