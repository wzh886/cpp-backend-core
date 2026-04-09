cmake_minimum_required(VERSION 3.20)

project(cpp_backend_core
    VERSION 0.1.0
    DESCRIPTION "Production-minded C++20 HTTP backend starter"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CPP_BACKEND_CORE_BUILD_TESTS "Build unit tests" ON)
option(CPP_BACKEND_CORE_ENABLE_WARNINGS "Enable strict compiler warnings" ON)

add_library(project_warnings INTERFACE)
if(CPP_BACKEND_CORE_ENABLE_WARNINGS)
    target_compile_options(project_warnings INTERFACE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Wconversion -Wshadow>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->)
endif()

add_library(cpp_backend_core_lib
    src/app/application.cpp
    src/config/app_config.cpp
    src/core/app_error.cpp
    src/http/http_server.cpp
    src/http/request_context.cpp
    src/logging/logger.cpp
)

add_library(cpp_backend_core::lib ALIAS cpp_backend_core_lib)

target_include_directories(cpp_backend_core_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(cpp_backend_core_lib PUBLIC project_warnings)

target_compile_definitions(cpp_backend_core_lib PRIVATE APP_VERSION="${PROJECT_VERSION}")

add_executable(cpp_backend_core src/main.cpp)
target_link_libraries(cpp_backend_core PRIVATE cpp_backend_core::lib)

include(GNUInstallDirs)
install(TARGETS cpp_backend_core RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES config/app.json DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/cpp-backend-core)

if(CPP_BACKEND_CORE_BUILD_TESTS)
    enable_testing()

    add_executable(config_test tests/config_test.cpp)
    target_link_libraries(config_test PRIVATE cpp_backend_core::lib)
    add_test(NAME config_test COMMAND config_test)

    add_executable(error_test tests/error_test.cpp)
    target_link_libraries(error_test PRIVATE cpp_backend_core::lib)
    add_test(NAME error_test COMMAND error_test)

    add_executable(request_context_test tests/request_context_test.cpp)
    target_link_libraries(request_context_test PRIVATE cpp_backend_core::lib)
    add_test(NAME request_context_test COMMAND request_context_test)
endif()
