cmake_minimum_required(VERSION 3.16)

project(http-server)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(http-server
    main.cpp
    router/router.cpp
    server/server.cpp
    http/http-method.cpp
    http/http-request.cpp
    http/http-response.cpp
)

target_include_directories(http-server PRIVATE
    .
)
