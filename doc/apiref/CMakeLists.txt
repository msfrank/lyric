find_package(Doxygen)

if (${DOXYGEN_FOUND})

    # src input directories
    set(APIREF_INPUT_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/pages"
        "${CMAKE_SOURCE_DIR}/lib/lyric_build"
        "${CMAKE_SOURCE_DIR}/lib/lyric_object"
        "${CMAKE_SOURCE_DIR}/lib/lyric_runtime"
        )

    list(JOIN APIREF_INPUT_DIRECTORIES " " APIREF_INPUT_DIRECTORIES)

    # HTML output directory
    set(APIREF_LAYOUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/DoxygenLayout.xml")

    # HTML output directory
    set(APIREF_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
        "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        @ONLY)

    add_custom_target(apiref ALL
        Doxygen::doxygen
            "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        DEPENDS
            "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        COMMENT "Building API Reference documentation"
        )

else()
    message(WARNING "Failed to find doxygen program, apiref will not be built")
endif()
