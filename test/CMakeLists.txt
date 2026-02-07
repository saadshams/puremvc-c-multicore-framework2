add_executable(ControllerTest core/controller_test.c
        core/controller_test_command.c
        core/controller_test_command2.c)
target_link_libraries(ControllerTest PRIVATE puremvc)
add_test(NAME PureMVC.ControllerTest COMMAND ControllerTest)

add_executable(ModelTest core/model_test.c core/model_test_proxy.c)
target_link_libraries(ModelTest PRIVATE puremvc)
add_test(NAME PureMVC.ModelTest COMMAND ModelTest)

add_executable(ViewTest core/view_test.c
        core/view_test_mediator.c core/view_test_mediator2.c
        core/view_test_mediator3.c core/view_test_mediator4.c
        core/view_test_mediator5.c core/view_test_mediator6.c
        core/view_test_mediator7.c)
target_link_libraries(ViewTest PRIVATE puremvc)
add_test(NAME PureMVC.ViewTest COMMAND ViewTest)

add_executable(SimpleCommandTest
        patterns/command/simple_command_test.c
        patterns/command/simple_command_test_command.c)
target_link_libraries(SimpleCommandTest PRIVATE puremvc)
add_test(NAME PureMVC.SimpleCommandTest COMMAND SimpleCommandTest)

add_executable(MacroCommandTest patterns/command/macro_command_test.c
        patterns/command/macro_command_test_command.c
        patterns/command/macro_command_test_command2.c
        patterns/command/macro_command_test_sub1_command.c
        patterns/command/macro_command_test_sub2_command.c
        patterns/command/macro_command_test_sub3_command.c)
target_link_libraries(MacroCommandTest PRIVATE puremvc)
add_test(NAME PureMVC.MacroCommandTest COMMAND MacroCommandTest)

#add_executable(FacadeTest patterns/facade/facade_test.c
#        patterns/facade/facade_test_command.c)
#target_link_libraries(FacadeTest PRIVATE puremvc)
#add_test(NAME PureMVC.FacadeTest COMMAND FacadeTest)

add_executable(MediatorTest patterns/mediator/mediator_test.c)
target_link_libraries(MediatorTest PRIVATE puremvc)
add_test(NAME PureMVC.MediatorTest COMMAND MediatorTest)

add_executable(NotificationTest patterns/observer/notification_test.c)
target_link_libraries(NotificationTest PRIVATE puremvc)
add_test(NAME PureMVC.NotificationTest COMMAND NotificationTest)

add_executable(NotifierTest patterns/observer/notifier_test.c)
target_link_libraries(NotifierTest PRIVATE puremvc)
add_test(NAME PureMVC.NotifierTest COMMAND NotifierTest)

add_executable(ObserverTest patterns/observer/observer_test.c)
target_link_libraries(ObserverTest PRIVATE puremvc)
add_test(NAME PureMVC.ObserverTest COMMAND ObserverTest)

add_executable(ProxyTest patterns/proxy/proxy_test.c)
target_link_libraries(ProxyTest PRIVATE puremvc)
add_test(NAME PureMVC.ProxyTest COMMAND ProxyTest)
