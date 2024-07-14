
# build lyric_rewriter as a shared library
add_library(lyric_rewriter SHARED)
add_library(lyric::lyric_rewriter ALIAS lyric_rewriter)

set(LYRIC_REWRITER_INCLUDES
    include/lyric_rewriter/lyric_rewriter.h
    include/lyric_rewriter/rewriter_result.h
    )
set_target_properties(lyric_rewriter PROPERTIES PUBLIC_HEADER "${LYRIC_REWRITER_INCLUDES}")

target_sources(lyric_rewriter PRIVATE
    src/lyric_rewriter.cpp
    src/rewriter_result.cpp

    include/lyric_rewriter/internal/entry_point.h
    src/internal/entry_point.cpp
    include/lyric_rewriter/internal/rewrite_archetype.h
    src/internal/rewrite_archetype.cpp
    )

# set the library version
set_target_properties(lyric_rewriter PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_rewriter PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_rewriter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_rewriter PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(lyric_rewriter
    PUBLIC
    lyric::lyric_assembler
    lyric::lyric_importer
    lyric::lyric_parser
    lyric::lyric_runtime
    lyric::lyric_typing
    tempo::tempo_utils
    absl::flat_hash_map
    absl::flat_hash_set
    PRIVATE
    absl::strings
    )

# install targets
install(TARGETS lyric_rewriter EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_rewriter
    )

# add testing subdirectory
#add_subdirectory(test)
