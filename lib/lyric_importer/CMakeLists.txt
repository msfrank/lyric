
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_importer as a shared library
add_library(lyric_importer SHARED)
add_library(lyric::lyric_importer ALIAS lyric_importer)

set(LYRIC_IMPORTER_INCLUDES
    include/lyric_importer/action_import.h
    include/lyric_importer/call_import.h
    include/lyric_importer/class_import.h
    include/lyric_importer/concept_import.h
    include/lyric_importer/enum_import.h
    include/lyric_importer/existential_import.h
    include/lyric_importer/field_import.h
    include/lyric_importer/impl_import.h
    include/lyric_importer/importer_result.h
    include/lyric_importer/importer_types.h
    include/lyric_importer/instance_import.h
    include/lyric_importer/module_cache.h
    include/lyric_importer/module_import.h
    include/lyric_importer/namespace_import.h
    include/lyric_importer/static_import.h
    include/lyric_importer/struct_import.h
    include/lyric_importer/template_import.h
    include/lyric_importer/type_import.h
    )
set_target_properties(lyric_importer PROPERTIES PUBLIC_HEADER "${LYRIC_IMPORTER_INCLUDES}")

target_sources(lyric_importer PRIVATE
    src/action_import.cpp
    src/call_import.cpp
    src/class_import.cpp
    src/concept_import.cpp
    src/enum_import.cpp
    src/existential_import.cpp
    src/field_import.cpp
    src/impl_import.cpp
    src/importer_result.cpp
    src/importer_types.cpp
    src/instance_import.cpp
    src/module_cache.cpp
    src/module_import.cpp
    src/namespace_import.cpp
    src/static_import.cpp
    src/struct_import.cpp
    src/template_import.cpp
    src/type_import.cpp
    )

# set the library version
set_target_properties(lyric_importer PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_importer PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_importer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_importer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(lyric_importer
    PUBLIC
    lyric::lyric_parser
    lyric::lyric_runtime
    tempo::tempo_tracing
    tempo::tempo_utils
    absl::flat_hash_map
    absl::flat_hash_set
    PRIVATE
    absl::strings
    flatbuffers::flatbuffers
    icu::icu
    )

# install targets
install(TARGETS lyric_importer EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_importer
    )

# add testing subdirectory
#add_subdirectory(test)
