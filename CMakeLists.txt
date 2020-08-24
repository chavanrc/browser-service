cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(BrowserStack
        DESCRIPTION "Browser Service Project"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    list(APPEND CMAKE_CXX_FLAGS "-Wno-deprecated -Wno-attributes")
endif()

message(STATUS "Build type set to ${CMAKE_BUILD_TYPE}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/external/cmake)

include(Versions)
include(ClangFormatBrowserService)
include(GNUInstallDirs)
set(CMAKE_NOOP ${CMAKE_COMMAND} -E echo)
set_property(DIRECTORY PROPERTY EP_BASE ${CMAKE_BINARY_DIR}/subprojects)
set(STAGED_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/stage)
message(STATUS "${PROJECT_NAME} staged install: ${STAGED_INSTALL_PREFIX}")

add_subdirectory(external/uwebsockets)
add_subdirectory(external/spdlog)
add_subdirectory(external/nlohmann)
add_subdirectory(external/catch)

include(ExternalProject)

ExternalProject_Add(service 
        DEPENDS
            uwebsockets_external
            spdlog_external
            nlohmann_external
        PREFIX
            ${STAGED_INSTALL_PREFIX}/service
        SOURCE_DIR
            ${CMAKE_CURRENT_LIST_DIR}/service
        CMAKE_ARGS
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=TRUE
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=TRUE
            -DCMAKE_INSTALL_PREFIX=${STAGED_INSTALL_PREFIX}/service
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
            -DCMAKE_CXX_EXTENSIONS=${CMAKE_CXX_EXTENSIONS}
            -DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
        CMAKE_CACHE_ARGS
            -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
            -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH};
                                     ${NLOHMANN_CONFIG_DIR};
            -DCMAKE_INCLUDE_PATH:PATH=${UWEBSOCKETS_INCLUDE_DIR};
        BUILD_ALWAYS
            1)

ExternalProject_Add(bs_tests
        DEPENDS
            catch_external
            service
        PREFIX
            ${STAGED_INSTALL_PREFIX}/bs_tests
        SOURCE_DIR
            ${CMAKE_CURRENT_LIST_DIR}/bs_tests
        CMAKE_ARGS
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=TRUE
            -DCMAKE_INSTALL_PREFIX=${STAGED_INSTALL_PREFIX}/bs_tests
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
            -DCMAKE_CXX_EXTENSIONS=${CMAKE_CXX_EXTENSIONS}
            -DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
        CMAKE_CACHE_ARGS
            -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
            -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH};
                                     ${CATCH_CONFIG_DIR};
                                     ${STAGED_INSTALL_PREFIX}/service/${CMAKE_INSTALL_LIBDIR}/cmake/BrowserService;
                                     ${NLOHMANN_CONFIG_DIR};
            -DCMAKE_INCLUDE_PATH:PATH=${CATCH_INCLUDE_DIR};
                                      ${STAGED_INSTALL_PREFIX}/service/include;
                                      ${NLOHMANN_INCLUDE_DIR};
                                      ${UWEBSOCKETS_INCLUDE_DIR};
            -DCMAKE_LIBRARY_PATH:PATH=${UWEBSOCKETS_LIB_PATH};
        BUILD_ALWAYS
            1)
