#include "facade.h"
#include "facade_test_command.h"
#include "facade_test_vo.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void testGetInstance() {
    // Test Factory Method
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey1", puremvc_facade_new);

    // test assertions
    assert(facade != NULL);
    puremvc_facade_removeFacade("FacadeTestKey1");
}

void testRegisterCommandAndSendNotification() {
    // Create the Facade, register the FacadeTestCommand to
    // handle 'FacadeTest' notifications
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey2", puremvc_facade_new);
    facade->registerCommand(facade, "FacadeTestNote", test_facade_command_new);

    // Send notification. The Command associated with the event
    // (FacadeTestCommand) will be invoked, and will multiply
    // the vo.input value by 2 and set the result on vo.result
    FacadeTestVO vo = {32};
    facade->sendNotification(facade, "FacadeTestNote", &vo, NULL);

    // test assertions
    assert(vo.result == 64);
    facade->removeCommand(facade, "FacadeTestNote");
    puremvc_facade_removeFacade("FacadeTestKey2");
}

void testRegisterAndRemoveCommandAndSendNotification() {
    // Create the Facade, register the FacadeTestCommand to
    // handle 'FacadeTest' events
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey3", puremvc_facade_new);
    facade->registerCommand(facade, "FacadeTestNote", test_facade_command_new);
    facade->removeCommand(facade, "FacadeTestNote");

    // Send notification. The Command associated with the event
    // (FacadeTestCommand) will NOT be invoked, and will NOT multiply
    // the vo.input value by 2
    FacadeTestVO vo = {32};
    facade->sendNotification(facade, "FacadeTestNote", &vo, NULL);

    // test assertions
    assert(vo.result != 64);
    puremvc_facade_removeFacade("FacadeTestKey3");
}

void testRegisterAndRetrieveProxy() {
    // register a new and retrieve it.
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey4", puremvc_facade_new);
    facade->registerProxy(facade, puremvc_proxy_new("colors", (char *[]) {"red", "green", "blue", NULL}));
    struct Proxy *proxy = facade->retrieveProxy(facade, "colors");

    // test assertions
    assert(proxy != NULL);

    // retrieve data from new
    const char **data = proxy->getData(proxy);

    // test assertions
    assert(data != NULL);
    assert(strcmp(data[0], "red") == 0);
    assert(strcmp(data[1], "green") == 0);
    assert(strcmp(data[2], "blue") == 0);

    struct Proxy *removedProxy = facade->removeProxy(facade, "colors");
    puremvc_proxy_free(&removedProxy);
    removedProxy = NULL;
    puremvc_facade_removeFacade("FacadeTestKey4");
}

void testRegisterAndRemoveProxy() {
    // register a new, remove it, then try to retrieve it
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey5", puremvc_facade_new);
    facade->registerProxy(facade, puremvc_proxy_new("sizes", (char *[]) {"7", "13", "21"}));

    // remove the new
    struct Proxy *removedProxy = facade->removeProxy(facade, "sizes");

    // assert that we removed the appropriate new
    assert(strcmp(removedProxy->getProxyName(removedProxy), "sizes") == 0);

    // test assertions - make sure we can no longer retrieve the new from the model
    assert(facade->retrieveProxy(facade, "sizes") == NULL);

    puremvc_proxy_free(&removedProxy);
    puremvc_facade_removeFacade("FacadeTestKey5");
}

void testRegisterRetrieveAndRemoveMediator() {
    // register a mediator, remove it, then try to retrieve it
    struct Object {} object;
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey6", puremvc_facade_new);
    facade->registerMediator(facade, puremvc_mediator_new(MEDIATOR_NAME, &object));

    // retrieve the mediator
    assert(facade->retrieveMediator(facade, MEDIATOR_NAME) != NULL);

    // remove the mediator
    struct Mediator *removedMediator = facade->removeMediator(facade, MEDIATOR_NAME);

    // assert that we have removed the appropriate mediator
    assert(strcmp(removedMediator->getMediatorName(removedMediator), MEDIATOR_NAME) == 0);

    // assert that the mediator is no longer retrievable
    assert(facade->retrieveMediator(facade, MEDIATOR_NAME) == NULL);
    puremvc_mediator_free(&removedMediator);
   puremvc_facade_removeFacade("FacadeTestKey6");
}

void testHasProxy() {
    // register a Proxy
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey7", puremvc_facade_new);
    facade->registerProxy(facade, puremvc_proxy_new("hasProxyTest", (int[]) {1, 2, 3}));

    // assert that the model.hasProxy method returns true
    // for that new name
    assert(facade->hasProxy(facade, "hasProxyTest") == true);

    struct Proxy *removedProxy = facade->removeProxy(facade, "hasProxyTest");
    puremvc_proxy_free(&removedProxy);
    assert(facade->hasProxy(facade, "hasProxyTest") == false);
    puremvc_facade_removeFacade("FacadeTestKey7");
}

void testHasMediator() {
    // register a Mediator
    struct Object {} object;
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey8", puremvc_facade_new);
    facade->registerMediator(facade, puremvc_mediator_new("facadeHasMediatorTest", &object));

    // assert that the facade.hasMediator method returns true
    // for that mediator name
    assert(facade->hasMediator(facade, "facadeHasMediatorTest") == true);

    struct Mediator *removedMediator = facade->removeMediator(facade, "facadeHasMediatorTest");
    puremvc_mediator_free(&removedMediator);

    // assert that the facade.hasMediator method returns false
    // for that mediator name
    assert(facade->hasMediator(facade, "facadeHasMediatorTest") == false);
    puremvc_facade_removeFacade("FacadeTestKey8");
}

void testHasCommand() {
    // register the ControllerTestCommand to handle 'hasCommandTest' notes
    struct Facade *facade = puremvc_facade_getInstance("FacadeTestKey10", puremvc_facade_new);
    facade->registerCommand(facade, "facadeHasCommandTest", (struct SimpleCommand *(*)()) test_facade_command_new);

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
    puremvc_facade_getInstance("FacadeTestKey11", puremvc_facade_new);

    assert(puremvc_facade_hasCore("FacadeTestKey11") == true);

    // remove the Core
    puremvc_facade_removeFacade("FacadeTestKey11");

    // assert that the Facade.hasCore method returns false now that the core has been removed.
    assert(puremvc_facade_hasCore("FacadeTestKey11") == false);
}

struct Facade *facade;

void *compareInstances() {
    assert(facade == puremvc_facade_getInstance("FacadeTestKey11", puremvc_facade_new));
    assert(puremvc_facade_hasCore("FacadeTestKey11") == true);
    return NULL;
}

void testGetInstancesThreaded() {
    facade = puremvc_facade_getInstance("FacadeTestKey11", puremvc_facade_new);

    int total = 100;
    pthread_t *thread_group = malloc(sizeof(pthread_t) * total);

    // start all threads to begin work
    for (int i = 0; i < total; i++) {
        int error = pthread_create(&thread_group[i], NULL, compareInstances, NULL);
        if (error != 0)
            printf("\nThread can't be created : [%s]", strerror(error));
    }

    // wait for all threads to finish
    for (int i = 0; i < total; i++) {
        pthread_join(thread_group[i], NULL);
    }

    free(thread_group);

    // cleanup
    puremvc_facade_removeFacade("FacadeTestKey11");
}

int main() {
    testGetInstance();
    testRegisterCommandAndSendNotification();
    testRegisterAndRemoveCommandAndSendNotification();
    testRegisterAndRetrieveProxy();
    testRegisterAndRemoveProxy();
    testRegisterRetrieveAndRemoveMediator();
    testHasProxy();
    testHasMediator();
    testHasCommand();
    testHasCoreAndRemoveCore();
    testGetInstancesThreaded();
    return 0;
}
