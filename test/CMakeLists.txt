add_executable(controller_test
        core/controller_test.c
        core/controller_test_command.c
        core/controller_test_command2.c
        core/controller_test_vo.c)
target_link_libraries(controller_test puremvc)
add_test(NAME TestController COMMAND controller_test)

add_executable(model_test
        core/model_test.c
        core/model_test_proxy.c)
target_link_libraries(model_test puremvc)
add_test(NAME TestModel COMMAND model_test)

add_executable(view_test
        core/view_test_component.c
        core/view_test.c
        core/view_test_mediator.c
        core/view_test_mediator2.c
        core/view_test_mediator3.c
        core/view_test_mediator4.c
        core/view_test_mediator5.c
        core/view_test_mediator6.c)
target_link_libraries(view_test puremvc)
add_test(NAME TestView COMMAND view_test)

add_executable(array_test patterns/collection/array_test.c)
target_link_libraries(array_test puremvc)
add_test(NAME TestArray COMMAND array_test)

add_executable(simple_command_test patterns/command/simple_command_test.c)
target_link_libraries(simple_command_test puremvc)
add_test(NAME TestSimpleCommand COMMAND simple_command_test)

add_executable(facade_test
        patterns/facade/facade_test.c
        patterns/facade/facade_test_command.c
        patterns/facade/facade_test_vo.c)
target_link_libraries(facade_test puremvc)
add_test(NAME TestFacade COMMAND facade_test)

add_executable(mediator_test patterns/mediator/mediator_test.c)
target_link_libraries(mediator_test puremvc)
add_test(NAME TestMediator COMMAND mediator_test)

add_executable(notification_test patterns/observer/notification_test.c)
target_link_libraries(notification_test puremvc)
add_test(NAME TestNotification COMMAND notification_test)

add_executable(observer_test patterns/observer/observer_test.c)
target_link_libraries(observer_test puremvc)
add_test(NAME TestObserver COMMAND observer_test)

add_executable(proxy_test patterns/proxy/proxy_test.c)
target_link_libraries(proxy_test puremvc)
add_test(NAME TestProxy COMMAND proxy_test)

add_executable(dictionary_test patterns/collection/dictionary_test.c)
target_link_libraries(dictionary_test puremvc)
add_test(NAME TestDictionary COMMAND dictionary_test)

add_executable(macro_command_test
        patterns/command/macro_command_test.c
        patterns/command/macro_command_test_vo.c
        patterns/command/macro_command_test_command.c
        patterns/command/macro_command_test_sub1command.c
        patterns/command/macro_command_test_sub2command.c)
target_link_libraries(macro_command_test puremvc)
add_test(NAME TestMacroCommand COMMAND macro_command_test)

add_executable(notifier_test
        patterns/facade/notifier_test.c)
target_link_libraries(notifier_test puremvc)
add_test(NAME TestNotifier COMMAND notifier_test)
