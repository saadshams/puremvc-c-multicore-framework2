cmake_minimum_required(VERSION 3.26)
project(puremvc C)

set(CMAKE_C_STANDARD 11)

add_library(puremvc
        src/core/controller.c
        src/core/model.c
        src/core/view.c
        src/patterns/collection/array.c
        src/patterns/collection/dictionary.c
        src/patterns/command/macro_command.c
        src/patterns/command/simple_command.c
        src/patterns/facade/facade.c
        src/patterns/facade/notifier.c
        src/patterns/mediator/mediator.c
        src/patterns/observer/notification.c
        src/patterns/observer/observer.c
        src/patterns/proxy/proxy.c
)
target_include_directories(puremvc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

enable_testing()
add_subdirectory(test)
