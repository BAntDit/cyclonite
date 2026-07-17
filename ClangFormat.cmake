
if (CLANG_FORMAT_PATH)
    set(CLANG_FORMAT "${CLANG_FORMAT_PATH}/clang-format")

    if(EXISTS "${CLANG_FORMAT}")
        message(STATUS "Using Conan clang-format: ${CLANG_FORMAT}")
    else()
        message(WARNING "Conan clang-format not found at: ${CLANG_FORMAT}")

        find_program(
            CLANG_FORMAT
            NAMES "clang-format"
            DOC "Path to clang-format executable"
        )
    endif()
else()
    find_program(
        CLANG_FORMAT
        NAMES "clang-format"
        DOC "Path to clang-format executable"
    )
endif()

macro(CODE_STYLE_CORRECTION FILES_TO_ADJUST_STYLE)
    set(CODE_STYLE Mozilla)

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format")
        set(CODE_STYLE file)
    endif()

    if (CLANG_FORMAT)
        add_custom_target(code-style-format ALL
                COMMAND "${CLANG_FORMAT}" -style=${CODE_STYLE} -i ${FILES_TO_ADJUST_STYLE}
        )
    else (CLANG_FORMAT)
        message("clang-format not found")
    endif()
endmacro()

