
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_analyzer as a shared library
add_library(lyric_analyzer SHARED)
add_library(lyric::lyric_analyzer ALIAS lyric_analyzer)

set(LYRIC_ANALYZER_INCLUDES
    include/lyric_analyzer/analyzer_result.h
    include/lyric_analyzer/lyric_analyzer.h
    )
set_target_properties(lyric_analyzer PROPERTIES PUBLIC_HEADER "${LYRIC_ANALYZER_INCLUDES}")

target_sources(lyric_analyzer PRIVATE
    src/analyzer_result.cpp
    src/lyric_analyzer.cpp

    include/lyric_analyzer/internal/analyze_def.h
    src/internal/analyze_def.cpp
    include/lyric_analyzer/internal/analyze_defclass.h
    src/internal/analyze_defclass.cpp
    include/lyric_analyzer/internal/analyze_module.h
    src/internal/analyze_module.cpp
    include/lyric_analyzer/internal/analyze_node.h
    src/internal/analyze_node.cpp
    include/lyric_analyzer/internal/entry_point.h
    src/internal/entry_point.cpp
    )

# set the library version
set_target_properties(lyric_analyzer PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_analyzer PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_analyzer PROPERTIES
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
#add_subdirectory(test)
