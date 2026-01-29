cmake_minimum_required(VERSION 3.31.6)
project(puremvc VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_library(puremvc STATIC
        src/core/model.c
        src/core/view.c
        src/patterns/command/simple_command.c
        src/patterns/command/macro_command.c
        src/patterns/mediator/mediator.c
        src/patterns/observer/notification.c
        src/patterns/observer/notifier.c
        src/patterns/observer/observer.c
        src/patterns/proxy/proxy.c)

add_library(puremvc::puremvc ALIAS puremvc)

# 4️⃣ Target include paths
target_include_directories(puremvc
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

# 9️⃣ Tests
option(BUILD_TESTS "Build Tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
