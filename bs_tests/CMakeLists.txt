cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(BSTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

list(APPEND CMAKE_MODULE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/../external/cmake)

find_package(PkgConfig REQUIRED)

pkg_search_module(OPENSSL REQUIRED openssl)

if (OPENSSL_FOUND)
    include_directories(include ${CMAKE_INCLUDE_PATH} ${OPENSSL_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
    message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
else ()
    # Error; with REQUIRED, pkg_search_module() will throw an error by it's own
endif ()

find_package(Catch2 REQUIRED CONFIG QUIET)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(uwebsockets REQUIRED QUIET)
find_package(nlohmann_json REQUIRED CONFIG QUIET)

set(SOURCE_FILES
        main.cpp
        service_tests.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} BrowserService::BrowserService
        Threads::Threads Catch2::Catch2 ZLIB::ZLIB uwebsockets::uwebsockets nlohmann_json::nlohmann_json)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

#target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
