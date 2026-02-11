cmake_minimum_required(VERSION 3.22)
project(puremvc VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_library(puremvc STATIC
        src/core/controller.c
        src/core/model.c
        src/core/view.c
        src/patterns/command/simple_command.c
        src/patterns/command/macro_command.c
        src/patterns/facade/facade.c
        src/patterns/mediator/mediator.c
        src/patterns/observer/notification.c
        src/patterns/observer/notifier.c
        src/patterns/observer/observer.c
        src/patterns/proxy/proxy.c)

add_library(puremvc::puremvc ALIAS puremvc)

if(WIN32)
    target_sources(puremvc PRIVATE src/platform/win/mutex.c)
else()
    target_sources(puremvc PRIVATE src/platform/posix/i_mutex.c)
endif()

# 4️⃣ Target include paths
target_include_directories(puremvc
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

# Compiler warnings (cross-platform, strict)
target_compile_options(puremvc
        PRIVATE
            $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Werror>
            $<$<C_COMPILER_ID:MSVC>:/W4 /WX>)

# 5️⃣ Install rules
include(GNUInstallDirs)

install(TARGETS puremvc
        EXPORT puremvc-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT PureMVCDevelopment)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# 6️⃣ Export targets for find_package()
install(EXPORT puremvc-targets
        FILE puremvc-targets.cmake
        NAMESPACE puremvc::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/puremvc)

# 7️⃣ Package configuration files
include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/puremvc-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/puremvc-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/puremvc)

# 8️⃣ Package version file
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/puremvc-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/puremvc-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/puremvc-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/puremvc)

# 9️⃣ Tests
option(BUILD_TESTS "Build Tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
