cmake_minimum_required(VERSION 3.21)

project(ft_ping VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(FT_PING_ENABLE_WARNINGS "Enable compiler warnings" ON)
option(FT_PING_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

set(FT_PING_PUBLIC_HEADERS
    include/ft_ping.h
    include/ft_ping/config.h
    include/ft_ping/cli/options.h
    include/ft_ping/core/session.h
    include/ft_ping/core/ping.h
    include/ft_ping/icmp/echo_utils.h
    include/ft_ping/icmp/echo.h
    include/ft_ping/icmp/echo_reply.h
    include/ft_ping/icmp/echo_request.h
    include/ft_ping/platform/socket.h
    include/ft_ping/platform/system.h
    include/ft_ping/platform/windows_support.h
    include/ft_ping/utils/logger.h
)

set(FT_PING_PRIVATE_SOURCES
    src/cli/options.c
    src/core/session.c
    src/core/ping.c
    src/icmp/echo_utils.c
    src/icmp/echo_reply.c
    src/icmp/echo_request.c
    src/platform/socket.c
    src/platform/system.c
    src/utils/logger.c
)

add_library(ft_ping_lib STATIC ${FT_PING_PRIVATE_SOURCES} ${FT_PING_PUBLIC_HEADERS})
add_library(ft_ping::lib ALIAS ft_ping_lib)

target_include_directories(ft_ping_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(ft_ping_lib
    PUBLIC FT_PING_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
           FT_PING_VERSION_MINOR=${PROJECT_VERSION_MINOR}
           FT_PING_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

if (WIN32)
    target_compile_definitions(ft_ping_lib PUBLIC FT_PING_PLATFORM_WINDOWS)
elseif (APPLE)
    target_compile_definitions(ft_ping_lib PUBLIC FT_PING_PLATFORM_DARWIN)
else()
    target_compile_definitions(ft_ping_lib PUBLIC FT_PING_PLATFORM_LINUX)
endif()

if (FT_PING_ENABLE_WARNINGS)
    if (MSVC)
        target_compile_options(ft_ping_lib PRIVATE /W4 /permissive- /w14242 /w14254 /w14263 /w14265 /w14287 /we4289 /w14296 /w14311 /w14826 /w14905 /w14906 /w14928)
        if (FT_PING_WARNINGS_AS_ERRORS)
            target_compile_options(ft_ping_lib PRIVATE /WX)
        endif()
    else()
        target_compile_options(ft_ping_lib PRIVATE -Wall -Wextra -Wpedantic)
        if (FT_PING_WARNINGS_AS_ERRORS)
            target_compile_options(ft_ping_lib PRIVATE -Werror)
        endif()
    endif()
endif()

if (WIN32)
    target_link_libraries(ft_ping_lib PUBLIC ws2_32 iphlpapi)
elseif (UNIX)
    find_library(M_LIBRARY m)
    if (M_LIBRARY)
        target_link_libraries(ft_ping_lib PUBLIC ${M_LIBRARY})
    endif()
endif()

add_executable(ft_ping src/main.c)
target_link_libraries(ft_ping PRIVATE ft_ping::lib)
set_target_properties(ft_ping PROPERTIES OUTPUT_NAME ft_ping)
