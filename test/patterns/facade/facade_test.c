#include "facade_test.h"
#include "../../../src/patterns/facade/facade.h"

#include "facade_test_command.h"
#include "facade_test_vo.h"

#include "puremvc/platform.h"
#include "puremvc/i_facade.h"
#include "puremvc/i_observer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void beforeAll() {}
static void beforeEach() {}
static void afterEach() {}
static void afterAll() {}

static void test(const char *name, void (*callback)()) {
    printf("\033[0;34m[RUNNING]\033[0m %s...\n", name);
    fflush(stdout);

    beforeEach();
    callback();
    afterEach();

    printf("\033[0;32m[PASSED]\033[0m %s\n", name);
    fflush(stdout);
}

void mytest() {
    struct ViewMap **viewMap = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct IView *view = puremvc_view_getInstance(viewMap, "FacadeTestKey0");

    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL };
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey0");
    facade->initializeFacade(facade, NULL, view, NULL);

    if (puremvc_facade_removeFacade("FacadeTestKey0", NULL) != true) abort();
    if (puremvc_facade_hasCore("FacadeTestKey0") != false) abort();
}

int main() {
    printf("\n\033[1;36m================================================\033[0m\n");
    printf("\033[1;36m[SUITE] %s\033[0m\n", "FacadeTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    beforeAll();
    // mytest();
    test("testGetInstance", testGetInstance);
    test("testRegisterCommandAndSendNotification", testRegisterCommandAndSendNotification);
    test("testRegisterAndRemoveCommandAndSendNotification", testRegisterAndRemoveCommandAndSendNotification);
    test("testRegisterAndRetrieveProxy", testRegisterAndRetrieveProxy);
    test("testRegisterAndRemoveProxy", testRegisterAndRemoveProxy);
    test("testRegisterRetrieveAndRemoveMediator", testRegisterRetrieveAndRemoveMediator);
    test("testHasProxy", testHasProxy);
    test("testHasMediator", testHasMediator);
    test("testHasCommand", testHasCommand);
    test("testHasCoreAndRemoveCore", testHasCoreAndRemoveCore);
    test("testFacadeMapShiftLeft", testFacadeMapShiftLeft);
    afterAll();

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
    if (facade == NULL) abort();
    if (facade != puremvc_facade_getInstance(instanceMap, "FacadeTestKey1")) abort();

    struct IFacade *removedFacade = NULL;
    if (puremvc_facade_removeFacade("FacadeTestKey1", &removedFacade) != true) abort();
    if (instanceMap[0]->key[0] != '\0') abort();
}

void testRegisterCommandAndSendNotification() {
    struct ViewMap **viewMap = (struct ViewMap *[]) { &(struct ViewMap){ .key = "", .view = alloca(puremvc_view_size()) }, NULL };
    struct ObserverMap **observerMap = (struct ObserverMap *[]) { &(struct ObserverMap) {
        .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } }, NULL
    };
    struct IView *view = puremvc_view_getInstance(viewMap, "FacadeTestKey2");
    view->initializeView(view, observerMap, NULL);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) { &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) }, NULL };
    struct CommandMap **commandMap = (struct CommandMap *[]) { &(struct CommandMap){}, NULL };
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "FacadeTestKey2");
    controller->initializeController(controller, view, commandMap);

    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL };

    // Create the Facade, register the FacadeTestCommand to
    // handle 'FacadeTest' notifications
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey2");
    facade->initializeFacade(facade, NULL, view, controller);

    if (facade->registerCommand(facade, "FacadeTestNote", test_facade_command) != true) abort();

    // Send notification. The Command associated with the event
    // (FacadeTestCommand) will be invoked, and will multiply
    // the vo.input value by 2 and set the result on vo.result
    struct FacadeTestVO vo = {32};
    facade->sendNotification(facade, "FacadeTestNote", &vo, NULL);

    // test assertions
    if (vo.result != 64) abort();

    struct ICommand *(*factory)(void *) = 0;
    if (facade->removeCommand(facade, "FacadeTestNote", &factory) != true) abort();

    struct IFacade *removedFacade = NULL;
    if (puremvc_facade_removeFacade("FacadeTestKey2", &removedFacade) != true) abort();
}

void testRegisterAndRemoveCommandAndSendNotification() {
    struct ViewMap **viewMap = (struct ViewMap *[]) { &(struct ViewMap){ .key = "", .view = alloca(puremvc_view_size()) }, NULL };
    struct ObserverMap **observerMap = (struct ObserverMap *[]) {
        &(struct ObserverMap){ .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        NULL
    };
    struct IView *view = puremvc_view_getInstance(viewMap, "FacadeTestKey3");
    view->initializeView(view, observerMap, NULL);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) { &(struct ControllerMap){ .key = "", .controller = alloca(puremvc_controller_size()) }, NULL };
    struct CommandMap **commandMap = (struct CommandMap *[]) { &(struct CommandMap){}, NULL };
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "FacadeTestKey3");
    controller->initializeController(controller, view, commandMap);

    // Create the Facade, register the FacadeTestCommand to
    // handle 'FacadeTest' events
    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .key = "", .facade = alloca(puremvc_facade_size()) }, NULL };
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey3");
    facade->initializeFacade(facade, NULL, view, controller);

    if (facade->registerCommand(facade, "FacadeTestNote", test_facade_command) != true) abort();
    if (facade->removeCommand(facade, "FacadeTestNote", NULL) != true) abort();

    // Send notification. The Command associated with the event
    // (FacadeTestCommand) will NOT be invoked, and will NOT multiply
    // the vo.input value by 2
    struct FacadeTestVO vo = {32};
    facade->sendNotification(facade, "FacadeTestNote", &vo, NULL);

    // test assertions
    if (vo.result != 0) abort();

    struct IFacade *removedFacade = NULL;
    if (puremvc_facade_removeFacade("FacadeTestKey3", &removedFacade) != true) abort();
}

void testRegisterAndRetrieveProxy() {
    struct ModelMap **modelMap = (struct ModelMap *[]) { &(struct ModelMap) { .model = alloca(puremvc_model_size()) }, NULL };
    struct ProxyMap **proxyMap = (struct ProxyMap *[]) { &(struct ProxyMap) { .proxy = alloca(puremvc_proxy_size()) }, NULL };
    struct IModel *model = puremvc_model_getInstance(modelMap, "FacadeTestKey4");
    model->initializeModel(model, proxyMap);

    // register a facade and retrieve it.
    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap) { .facade = alloca(puremvc_facade_size()) }, NULL};
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey4");
    facade->initializeFacade(facade, model, NULL, NULL);

    static char *colors[] = { "red", "green", "blue", NULL};

    if (facade->registerProxy(facade, puremvc_proxy_init, "colors", colors) != true) abort();
    const struct IProxy *proxy = facade->retrieveProxy(facade, "colors");

    // test assertions
    if (proxy == NULL) abort();

    // retrieve data from proxy
    const char **data = proxy->getData(proxy);

    // test assertions
    if (data == NULL) abort();
    if (strcmp(data[0], "red") != 0) abort();
    if (strcmp(data[1], "green") != 0) abort();
    if (strcmp(data[2], "blue") != 0) abort();

    struct IProxy *removedProxy = NULL;
    if (facade->removeProxy(facade, "colors", &removedProxy) == false) abort();

    struct IFacade *removedFacade = NULL;
    if (puremvc_facade_removeFacade("FacadeTestKey4", &removedFacade) != true) abort();
}

void testRegisterAndRemoveProxy() {
    struct ModelMap **modelMap = (struct ModelMap *[]) { &(struct ModelMap) { .model = alloca(puremvc_model_size()) }, NULL };
    struct ProxyMap **proxyMap = (struct ProxyMap *[]) { &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) }, NULL };
    struct IModel *model = puremvc_model_getInstance(modelMap, "FacadeTestKey5");
    model->initializeModel(model, proxyMap);

    struct ViewMap **viewMap = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct IView *view = puremvc_view_getInstance(viewMap, "FacadeTestKey5");

    // struct ControllerMap **controllerMap = (struct ControllerMap *[]){ &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) }, NULL };
    // struct IController *controller = puremvc_controller_getInstance(controllerMap, "FacadeTestKey5"); // extra to prevent memory aliasing

    // register a proxy, remove it, then try to retrieve it
    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap) { .facade = alloca(puremvc_facade_size()) }, NULL};
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey5");
    facade->initializeFacade(facade, model, view, NULL);

    static int sizes[] = { 7, 13, 21, 0 }; // 0 is the sentinel
    if (facade->registerProxy(facade, puremvc_proxy_init, "sizes", sizes) == false) abort();

    const struct IProxy *proxy = facade->retrieveProxy(facade, "sizes");

    // 1. Retrieve as the raw pointer (void * becomes int *)
    int *data = proxy->getData(proxy);

    // 2. Test assertions
    if (data == NULL) abort();
    if (data[0] != 7) abort();
    if (data[1] != 13) abort();
    if (data[2] != 21) abort();
    if (data[3] != 0) abort(); // Checking the sentinel

    // remove the proxy
    struct IProxy *removedProxy = NULL;
    if (facade->removeProxy(facade, "sizes", &removedProxy) == false) abort();

    // assert that we removed the appropriate proxy
    if (strcmp(removedProxy->getName(removedProxy), "sizes") != 0) abort();

    // test assertions - make sure we can no longer retrieve the proxy from the model
    if (facade->retrieveProxy(facade, "sizes") != NULL) abort();

    struct IFacade *removedFacade = NULL;
    if (puremvc_facade_removeFacade("FacadeTestKey5", &removedFacade) == false) abort();
}

void testRegisterRetrieveAndRemoveMediator() {
    struct ViewMap **viewMap = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct MediatorMap **mediatorMap = (struct MediatorMap *[]){ &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) }, NULL };
    struct IView *view = puremvc_view_getInstance(viewMap, "FacadeTestKey6");
    view->initializeView(view, NULL, mediatorMap);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]){ &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) }, NULL };
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "FacadeTestKey6");

    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL};
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey6");
    facade->initializeFacade(facade, NULL, view, controller);

    // register a mediator, remove it, then try to retrieve it
    struct Object {int x;} object;
    if (facade->registerMediator(facade, puremvc_mediator_init, MEDIATOR_NAME, &object) != true) abort();

    // retrieve the mediator
    if (facade->retrieveMediator(facade, MEDIATOR_NAME) == NULL) abort();

    // remove the mediator
    struct IMediator *removedMediator = NULL;
    if (facade->removeMediator(facade, MEDIATOR_NAME, &removedMediator) != true) abort();

    // assert that we have removed the appropriate mediator
    if (strcmp(removedMediator->getName(removedMediator), MEDIATOR_NAME) != 0) abort();

    // assert that the mediator is no longer retrievable
    if (facade->retrieveMediator(facade, MEDIATOR_NAME) != NULL) abort();

    struct IFacade *removedFacade = NULL;
    if (puremvc_facade_removeFacade("FacadeTestKey6", &removedFacade) != true) abort();
}

void testHasProxy() {
    struct ModelMap **modelMap = (struct ModelMap *[]) { &(struct ModelMap){ .model = alloca(puremvc_model_size()) }, NULL };
    struct ProxyMap **proxyMap = (struct ProxyMap *[]) { &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) }, NULL };
    struct IModel *model = puremvc_model_getInstance(modelMap, "FacadeTestKey7");
    model->initializeModel(model, proxyMap);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]){ &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) }, NULL };
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "FacadeTestKey7");

    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL};
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey7");
    facade->initializeFacade(facade, model, NULL, controller);

    static int sizes[] = { 7, 13, 21, 0 }; // 0 is the sentinel

    // register a Proxy
    if (facade->registerProxy(facade, puremvc_proxy_init, "hasProxyTest", sizes) != true) abort();

    // assert that the model.hasProxy method returns true
    // for that new name
    if (facade->hasProxy(facade, "hasProxyTest") != true) abort();

    if (facade->removeProxy(facade, "hasProxyTest", NULL) != true) abort();;
    if (facade->hasProxy(facade, "hasProxyTest") != false) abort();

    if (puremvc_facade_removeFacade("FacadeTestKey7", NULL) != true) abort();
}

void testHasMediator() {
    struct ViewMap **viewMap = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct MediatorMap **mediatorMap = (struct MediatorMap *[]){ &(struct MediatorMap){ .mediator = alloca(puremvc_mediator_size()) }, NULL };
    struct IView *view = puremvc_view_getInstance(viewMap, "FacadeTestKey8");
    view->initializeView(view, NULL, mediatorMap);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]){ &(struct ControllerMap){ .controller = alloca(puremvc_controller_size()) }, NULL };
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "FacadeTestKey8");

    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL};
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey8");
    facade->initializeFacade(facade, NULL, view, controller);

    // register a Mediator
    struct Object {int x;} object;
    if (facade->registerMediator(facade, puremvc_mediator_init, "facadeHasMediatorTest", &object) != true)
        abort();

    // assert that the facade.hasMediator method returns true
    // for that mediator name
    if (facade->hasMediator(facade, "facadeHasMediatorTest") != true) abort();

    if (facade->removeMediator(facade, "facadeHasMediatorTest", NULL) != true) abort();

    // assert that the facade.hasMediator method returns false
    // for that mediator name
    if (facade->hasMediator(facade, "facadeHasMediatorTest") != false) abort();
    if (puremvc_facade_removeFacade("FacadeTestKey8", NULL) != true) abort();
}

void testHasCommand() {
    struct ViewMap **viewMap = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct ObserverMap **observerMap = (struct ObserverMap *[]) { &(struct ObserverMap) {
        .observers = (struct IObserver *[]){ memset(alloca(puremvc_observer_size()), 0, puremvc_observer_size()), NULL } },
        NULL
    };
    struct IView *view = puremvc_view_getInstance(viewMap, "FacadeTestKey9");
    view->initializeView(view, observerMap, NULL);

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) { &(struct ControllerMap) { .controller = alloca(puremvc_controller_size()) }, NULL };
    struct CommandMap **commandMap = (struct CommandMap *[]) { &(struct CommandMap){}, NULL };
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "FacadeTestKey9");
    controller->initializeController(controller, view, commandMap);

    struct ModelMap **modelMap = (struct ModelMap *[]) { &(struct ModelMap){ .model = alloca(puremvc_model_size()) }, NULL };
    struct IModel *model = puremvc_model_getInstance(modelMap, "FacadeTestKey9");

    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL };
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey9");
    facade->initializeFacade(facade, model, view, controller);

    // register the ControllerTestCommand to handle 'hasCommandTest' notes
    if (facade->registerCommand(facade, "facadeHasCommandTest", test_facade_command) != true) abort();

    // test that hasCommand returns true for hasCommandTest notifications
    if (facade->hasCommand(facade, "facadeHasCommandTest") != true) abort();

    // Remove the Command from the Controller
    struct ICommand *(*removedCommand)(void *) = 0;
    if (facade->removeCommand(facade, "facadeHasCommandTest", &removedCommand) != true) abort();
    if (removedCommand != test_facade_command) abort();

    // test that hasCommand returns false for hasCommandTest notifications
    if (facade->hasCommand(facade, "facadeHasCommandTest") != false) abort();

    if (puremvc_facade_removeFacade("FacadeTestKey9", NULL) != true) abort();
}

void testHasCoreAndRemoveCore() {
    struct ViewMap **viewMap = (struct ViewMap *[]) { &(struct ViewMap){ .view = alloca(puremvc_view_size()) }, NULL };
    struct IView *view = puremvc_view_getInstance(viewMap, "FacadeTestKey10");

    struct ControllerMap **controllerMap = (struct ControllerMap *[]) { &(struct ControllerMap) { .controller = alloca(puremvc_controller_size()) }, NULL };
    struct IController *controller = puremvc_controller_getInstance(controllerMap, "FacadeTestKey10");

    struct ModelMap **modelMap = (struct ModelMap *[]) { &(struct ModelMap){ .model = alloca(puremvc_model_size()) }, NULL };
    struct IModel *model = puremvc_model_getInstance(modelMap, "FacadeTestKey10");

    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) }, NULL };
    struct IFacade *facade = puremvc_facade_getInstance(facadeMap, "FacadeTestKey10");
    facade->initializeFacade(facade, model, view, controller);

    // assert that the Facade.hasCore method returns false first
    if (puremvc_facade_hasCore("unregistered") != false) abort();

    // register a Core
    puremvc_facade_getInstance(facadeMap, "FacadeTestKey10");

    if (puremvc_facade_hasCore("FacadeTestKey10") != true) abort();

    // remove the Core
    puremvc_facade_removeFacade("FacadeTestKey10", NULL);

    // assert that the Facade.hasCore method returns false now that the core has been removed.
    if (puremvc_facade_hasCore("FacadeTestKey10") != false) abort();
}

void testFacadeMapShiftLeft() {
    struct FacadeMap **facadeMap = (struct FacadeMap *[]) { // facadeMap with 4 slots for the instance
        &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) },
        &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) },
        &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) },
        &(struct FacadeMap){ .facade = alloca(puremvc_facade_size()) },
        NULL
    };

    if (puremvc_facade_getInstance(facadeMap, "facade0") == NULL) abort();
    if (strcmp(facadeMap[0]->key, "facade0") != 0) abort();
    const char *key0 = (char *)facadeMap[0]->facade + sizeof(struct IFacade);
    if (strcmp(key0, "facade0") != 0) abort();
    if (puremvc_facade_hasCore("facade0") != true) abort();

    if (puremvc_facade_getInstance(facadeMap, "facade1") == NULL) abort();
    if (strcmp(facadeMap[1]->key, "facade1") != 0) abort();
    const char *key1 = (char *)facadeMap[1]->facade + sizeof(struct IFacade);
    if (strcmp(key1, "facade1") != 0) abort();
    if (puremvc_facade_hasCore("facade1") != true) abort();

    if (puremvc_facade_getInstance(facadeMap, "facade2") == NULL) abort();
    if (strcmp(facadeMap[2]->key, "facade2") != 0) abort();
    const char *key2 = (char *)facadeMap[2]->facade + sizeof(struct IFacade);
    if (strcmp(key2, "facade2") != 0) abort();
    if (puremvc_facade_hasCore("facade2") != true) abort();

    if (puremvc_facade_getInstance(facadeMap, "facade3") == NULL) abort();
    if (strcmp(facadeMap[3]->key, "facade3") != 0) abort();
    const char *key3 = (char *)facadeMap[3]->facade + sizeof(struct IFacade);
    if (strcmp(key3, "facade3") != 0) abort();
    if (puremvc_facade_hasCore("facade3") != true) abort();

    // remove
    struct IFacade *facade1 = NULL; // remove middle1, remaining 0, 2, 3
    if (puremvc_facade_removeFacade("facade1", &facade1) != true) abort();
    if (strcmp(facadeMap[0]->key, "facade0") != 0) abort();
    if (strcmp(facadeMap[1]->key, "facade2") != 0) abort();
    if (strcmp(facadeMap[2]->key, "facade3") != 0) abort();
    if (facadeMap[3]->key[0] != '\0') abort();
    if (facadeMap[4] != NULL) abort();
    if (puremvc_facade_hasCore("facade1") != false) abort();

    struct IFacade *facade3 = NULL; // remove last3, remaining 0, 2
    if (puremvc_facade_removeFacade("facade3", &facade3) != true) abort();
    if (strcmp(facadeMap[0]->key, "facade0") != 0) abort();
    if (strcmp(facadeMap[1]->key, "facade2") != 0) abort();
    if (facadeMap[2]->key[0] != '\0') abort();
    if (facadeMap[3]->key[0] != '\0') abort();
    if (facadeMap[4] != NULL) abort();
    if (puremvc_facade_hasCore("facade3") != false) abort();

    struct IFacade *facade0 = NULL; // remove first, remaining 2
    if (puremvc_facade_removeFacade("facade0", &facade0) != true) abort();
    if (strcmp(facadeMap[0]->key, "facade2") != 0) abort();
    if (facadeMap[1]->key[0] != '\0') abort();
    if (facadeMap[2]->key[0] != '\0') abort();
    if (facadeMap[3]->key[0] != '\0') abort();
    if (facadeMap[4] != NULL) abort();
    if (puremvc_facade_hasCore("facade0") != false) abort();

    struct IFacade *facade2 = NULL; // remove remaining
    if (puremvc_facade_removeFacade("facade2", &facade2) != true) abort();
    if (facadeMap[0]->key[0] != '\0') abort();
    if (facadeMap[1]->key[0] != '\0') abort();
    if (facadeMap[2]->key[0] != '\0') abort();
    if (facadeMap[3]->key[0] != '\0') abort();
    if (facadeMap[4] != NULL) abort();
    if (puremvc_facade_hasCore("facade2") != false) abort();
}
