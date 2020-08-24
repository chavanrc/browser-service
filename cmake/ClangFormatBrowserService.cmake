find_program(CLANG_FORMAT
        NAMES "clang-format"
        DOC "Path to clang-format executable")

if (NOT CLANG_FORMAT)
    message(STATUS "clang-format not found.")
else ()
    message(STATUS "clang-format found: ${CLANG_FORMAT}")
    set(DO_CLANG_FORMAT "${CLANG_FORMAT}" "-i -style=file")
endif ()

if (CLANG_FORMAT)
    # get all project files
    file(GLOB_RECURSE ALL_SOURCE_FILES
            RELATIVE ${CMAKE_CURRENT_BINARY_DIR}
            ${CMAKE_SOURCE_DIR}/procsmon/*.cpp ${CMAKE_SOURCE_DIR}/procsmon/*.hpp ${CMAKE_SOURCE_DIR}/procsmon/*.inl
            ${CMAKE_SOURCE_DIR}/execution/*.cpp ${CMAKE_SOURCE_DIR}/execution/*.hpp ${CMAKE_SOURCE_DIR}/execution/*.inl
            ${CMAKE_SOURCE_DIR}/hs_tests/*.cpp ${CMAKE_SOURCE_DIR}/hs_tests/*.hpp ${CMAKE_SOURCE_DIR}/hs_tests/*.inl
            ${CMAKE_SOURCE_DIR}/kafka_consumer/*.cpp ${CMAKE_SOURCE_DIR}/kafka_consumer/*.hpp ${CMAKE_SOURCE_DIR}/kafka_consumer/*.inl
            ${CMAKE_SOURCE_DIR}/tools/*.cpp ${CMAKE_SOURCE_DIR}/tools/*.hpp ${CMAKE_SOURCE_DIR}/tools/*.inl
            ${CMAKE_SOURCE_DIR}/common/*.cpp ${CMAKE_SOURCE_DIR}/common/*.hpp ${CMAKE_SOURCE_DIR}/common/*.inl
            ${CMAKE_SOURCE_DIR}/prometheus_cli/*.cpp ${CMAKE_SOURCE_DIR}/prometheus_cli/*.hpp ${CMAKE_SOURCE_DIR}/prometheus_cli/*.inl)

    add_custom_target(
            clang-format-hs
            COMMAND ${CLANG_FORMAT} -style=file -i ${ALL_SOURCE_FILES})

    add_custom_target(
            clang-format-diff-hs
            COMMAND ${CLANG_FORMAT} -style=file -i ${ALL_SOURCE_FILES}
            COMMAND git diff ${ALL_SOURCE_FILES}
            COMMENT "Formatting with clang-format (using ${CLANG_FORMAT}) and showing differences with latest commit"
    )
endif ()