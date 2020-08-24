cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(BrowserService LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../external/cmake)

find_package(PkgConfig REQUIRED)

pkg_search_module(OPENSSL REQUIRED openssl)

if (OPENSSL_FOUND)
    include_directories(include ${CMAKE_INCLUDE_PATH} ${OPENSSL_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
    message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
else ()
    # Error; with REQUIRED, pkg_search_module() will throw an error by it's own
endif ()

find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED CONFIG QUIET)

file(GLOB HEADER_FILES include/*.hpp)
file(GLOB SOURCE_FILES src/*.cpp)

target_link_libraries(${PROJECT_NAME} Threads::Threads ZLIB::ZLIB nlohmann_json::nlohmann_json ${OPENSSL_LIBRARIES})

target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

set_target_properties(${PROJECT_NAME} PROPERTIES
        PUBLIC_HEADER "${HEADER_FILES}")

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ${PROJECT_NAME}-targets
        NAMESPACE ${PROJECT_NAME}::
        FILE ${PROJECT_NAME}Config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
