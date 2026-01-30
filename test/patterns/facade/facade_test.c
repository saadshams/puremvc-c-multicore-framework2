#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "puremvc/facade.h"

#include "facade_test.h"
#include "facade_test_command.h"
#include "facade_test_vo.h"

int main() {
    // testGetInstance();
    // testRegisterCommandAndSendNotification();
    // testRegisterAndRemoveCommandAndSendNotification();
    // testRegisterAndRetrieveProxy();
    // testRegisterAndRemoveProxy();
    testRegisterRetrieveAndRemoveMediator();
    testHasProxy();
    testHasMediator();
    testHasCommand();
    testHasCoreAndRemoveCore();
    return 0;
}

void testGetInstance() {
    // Test Factory Method
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey1", puremvc_facade);
    facade->initializeFacade(facade);

    // test assertions
    assert(facade != NULL);
    puremvc_facade_removeFacade("FacadeTestKey1");
}

void testRegisterCommandAndSendNotification() {
    // Create the Facade, register the FacadeTestCommand to
    // handle 'FacadeTest' notifications
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey2", puremvc_facade);
    facade->initializeFacade(facade);
    facade->registerCommand(facade, "FacadeTestNote", test_facade_command);

    // Send notification. The Command associated with the event
    // (FacadeTestCommand) will be invoked, and will multiply
    // the vo.input value by 2 and set the result on vo.result
    struct FacadeTestVO *vo = malloc(sizeof(struct FacadeTestVO));
    vo->input = 32;
    facade->sendNotification(facade, "FacadeTestNote", vo, NULL);

    // test assertions
    assert(vo->result == 64);
    facade->removeCommand(facade, "FacadeTestNote");
    puremvc_facade_removeFacade("FacadeTestKey2");
}

void testRegisterAndRemoveCommandAndSendNotification() {
    // Create the Facade, register the FacadeTestCommand to
    // handle 'FacadeTest' events
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey3", puremvc_facade);
    facade->initializeFacade(facade);
    facade->registerCommand(facade, "FacadeTestNote", test_facade_command);
    facade->removeCommand(facade, "FacadeTestNote");

    // Send notification. The Command associated with the event
    // (FacadeTestCommand) will NOT be invoked, and will NOT multiply
    // the vo.input value by 2
    struct FacadeTestVO *vo = malloc(sizeof(struct FacadeTestVO));
    *vo = (struct FacadeTestVO) {32};
    facade->sendNotification(facade, "FacadeTestNote", vo, NULL);

    // test assertions
    assert(vo->result == 0);
    puremvc_facade_removeFacade("FacadeTestKey3");
}

void testRegisterAndRetrieveProxy() {
    // register a new and retrieve it.
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey4", puremvc_facade);
    facade->initializeFacade(facade);

    const char **colors = malloc(sizeof(char*) * 4);
    memset(colors, 0, sizeof(char*) * 4);
    for(const char **data = (const char *[]) {"red", "green", "blue", NULL}, **cursor = colors; *data; data++, cursor++) {
        *cursor = strdup(*data);
    }

    facade->registerProxy(facade, puremvc_proxy("colors", colors));
    const struct Proxy *proxy = facade->retrieveProxy(facade, "colors");

    // test assertions
    assert(proxy != NULL);

    // retrieve data from new
    const char **data = proxy->getData(proxy);

    // test assertions
    assert(data != NULL);
    assert(strcmp(data[0], "red") == 0);
    assert(strcmp(data[1], "green") == 0);
    assert(strcmp(data[2], "blue") == 0);

    struct Proxy removedProxy = facade->removeProxy(facade, "colors");
    proxy = NULL;
    puremvc_facade_removeFacade("FacadeTestKey4");
}

void testRegisterAndRemoveProxy() {
    // register a new, remove it, then try to retrieve it
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey5", puremvc_facade);
    facade->initializeFacade(facade);
    const char **sizes = malloc(sizeof(char*) * 4);
    memset(sizes, 0, sizeof(char*) * 4);
    for(const char **data = (const char *[]) {"7", "13", "21", NULL}, **cursor = sizes; *data; data++, cursor++) {
        *cursor = strdup(*data);
    }

    facade->registerProxy(facade, puremvc_proxy("sizes", sizes));

    // remove the new
    struct Proxy removedProxy = facade->removeProxy(facade, "sizes");

    // assert that we removed the appropriate new
    assert(strcmp(removedProxy.getName(&removedProxy), "sizes") == 0);

    // test assertions - make sure we can no longer retrieve the new from the model
    assert(facade->retrieveProxy(facade, "sizes") == NULL);

    puremvc_facade_removeFacade("FacadeTestKey5");
}

void testRegisterRetrieveAndRemoveMediator() {
    // register a mediator, remove it, then try to retrieve it
    struct Object {int x;} object;
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey6", puremvc_facade);
    facade->initializeFacade(facade);

    facade->registerMediator(facade, puremvc_mediator(MEDIATOR_NAME, &object));

    // retrieve the mediator
    assert(facade->retrieveMediator(facade, MEDIATOR_NAME) != NULL);

    // remove the mediator
    struct Mediator removedMediator = facade->removeMediator(facade, MEDIATOR_NAME);

    // assert that we have removed the appropriate mediator
    assert(strcmp(removedMediator.getName(&removedMediator), MEDIATOR_NAME) == 0);

    // assert that the mediator is no longer retrievable
    assert(facade->retrieveMediator(facade, MEDIATOR_NAME) == NULL);
   puremvc_facade_removeFacade("FacadeTestKey6");
}

void testHasProxy() {
    // register a Proxy
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey7", puremvc_facade);
    facade->initializeFacade(facade);

    int *sizes = malloc(sizeof(int) * 4), *cursor = sizes;
    memset(sizes, 0, sizeof(int) * 4);
    for(const int *data = (int []) {1, 2, 3, 0}; *data != 0; data++, cursor++) // 0 as terminator, or use -1
        *cursor = *data;

    facade->registerProxy(facade, puremvc_proxy("hasProxyTest", sizes));

    // assert that the model.hasProxy method returns true
    // for that new name
    assert(facade->hasProxy(facade, "hasProxyTest") == true);

    struct Proxy removedProxy = facade->removeProxy(facade, "hasProxyTest");
    assert(facade->hasProxy(facade, "hasProxyTest") == false);
    puremvc_facade_removeFacade("FacadeTestKey7");
}

void testHasMediator() {
    // register a Mediator
    struct Object {int x;} object;
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey8", puremvc_facade);
    facade->initializeFacade(facade);

    facade->registerMediator(facade, puremvc_mediator("facadeHasMediatorTest", &object));

    // assert that the facade.hasMediator method returns true
    // for that mediator name
    assert(facade->hasMediator(facade, "facadeHasMediatorTest") == true);

    facade->removeMediator(facade, "facadeHasMediatorTest");

    // assert that the facade.hasMediator method returns false
    // for that mediator name
    assert(facade->hasMediator(facade, "facadeHasMediatorTest") == false);
    puremvc_facade_removeFacade("FacadeTestKey8");
}

void testHasCommand() {
    // register the ControllerTestCommand to handle 'hasCommandTest' notes
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey10", puremvc_facade);
    facade->initializeFacade(facade);
    facade->registerCommand(facade, "facadeHasCommandTest", test_facade_command);

    // test that hasCommand returns true for hasCommandTest notifications
    assert(facade->hasCommand(facade, "facadeHasCommandTest") == true);

    // Remove the Command from the Controller
    facade->removeCommand(facade, "facadeHasCommandTest");

    // test that hasCommand returns false for hasCommandTest notifications
    assert(facade->hasCommand(facade, "facadeHasCommandTest") == false);
    puremvc_facade_removeFacade("FacadeTestKey10");
}

void testHasCoreAndRemoveCore() {
    // assert that the Facade.hasCore method returns false first
    assert(puremvc_facade_hasCore("FacadeTestKey11") == false);

    // register a Core
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey11", puremvc_facade);
    facade->initializeFacade(facade);

    assert(puremvc_facade_hasCore("FacadeTestKey11") == true);

    // remove the Core
    puremvc_facade_removeFacade("FacadeTestKey11");

    // assert that the Facade.hasCore method returns false now that the core has been removed.
    assert(puremvc_facade_hasCore("FacadeTestKey11") == false);
}
