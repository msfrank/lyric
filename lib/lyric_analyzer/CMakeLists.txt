
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_analyzer as a shared library
add_library(lyric_analyzer SHARED)
add_library(lyric::lyric_analyzer ALIAS lyric_analyzer)

set(LYRIC_ANALYZER_INCLUDES
    include/lyric_analyzer/abstract_analyzer_context.h
    include/lyric_analyzer/analyzer_result.h
    include/lyric_analyzer/analyzer_scan_driver.h
    include/lyric_analyzer/class_analyzer_context.h
    include/lyric_analyzer/concept_analyzer_context.h
    include/lyric_analyzer/entry_analyzer_context.h
    include/lyric_analyzer/enum_analyzer_context.h
    include/lyric_analyzer/impl_analyzer_context.h
    include/lyric_analyzer/import_analyzer_context.h
    include/lyric_analyzer/instance_analyzer_context.h
    include/lyric_analyzer/lyric_analyzer.h
    include/lyric_analyzer/namespace_analyzer_context.h
    include/lyric_analyzer/proc_analyzer_context.h
    include/lyric_analyzer/struct_analyzer_context.h
    )
set_target_properties(lyric_analyzer PROPERTIES PUBLIC_HEADER "${LYRIC_ANALYZER_INCLUDES}")

target_sources(lyric_analyzer PRIVATE
    src/analyzer_result.cpp
    src/analyzer_scan_driver.cpp
    src/class_analyzer_context.cpp
    src/concept_analyzer_context.cpp
    src/entry_analyzer_context.cpp
    src/enum_analyzer_context.cpp
    src/impl_analyzer_context.cpp
    src/import_analyzer_context.cpp
    src/instance_analyzer_context.cpp
    src/lyric_analyzer.cpp
    src/namespace_analyzer_context.cpp
    src/proc_analyzer_context.cpp
    src/struct_analyzer_context.cpp

    include/lyric_analyzer/internal/analyzer_utils.h
    src/internal/analyzer_utils.cpp
    )

# set the library version
set_target_properties(lyric_analyzer PROPERTIES VERSION "${PROJECT_VERSION}" SOVERSION "${PROJECT_VERSION_MAJOR}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_analyzer PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_analyzer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_analyzer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(lyric_analyzer
    PUBLIC
    lyric::lyric_importer
    lyric::lyric_parser
    lyric::lyric_rewriter
    lyric::lyric_runtime
    lyric::lyric_typing
    tempo::tempo_utils
    absl::flat_hash_map
    absl::flat_hash_set
    PRIVATE
    absl::strings
    )

# install targets
install(TARGETS lyric_analyzer EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_analyzer
    )

# add testing subdirectory
add_subdirectory(test)
