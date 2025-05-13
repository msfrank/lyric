
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_common as a shared library
add_library(lyric_common SHARED)
add_library(lyric::lyric_common ALIAS lyric_common)

set(LYRIC_COMMON_INCLUDES
    include/lyric_common/module_location.h
    include/lyric_common/common_conversions.h
    include/lyric_common/common_serde.h
    include/lyric_common/common_types.h
    include/lyric_common/plugin.h
    include/lyric_common/runtime_policy.h
    include/lyric_common/symbol_path.h
    include/lyric_common/symbol_url.h
    include/lyric_common/type_def.h
    )
set_target_properties(lyric_common PROPERTIES PUBLIC_HEADER "${LYRIC_COMMON_INCLUDES}")

target_sources(lyric_common
    PRIVATE
    src/module_location.cpp
    src/common_conversions.cpp
    src/common_serde.cpp
    src/common_types.cpp
    src/plugin.cpp
    src/runtime_policy.cpp
    src/symbol_path.cpp
    src/symbol_url.cpp
    src/type_def.cpp
    )

# set the library version
set_target_properties(lyric_common PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_common PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_common PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_compile_definitions(lyric_common PRIVATE
    "PLUGIN_SYSTEM_NAME=\"${CMAKE_SYSTEM_NAME}\""
    "PLUGIN_ARCHITECTURE=\"${CMAKE_SYSTEM_PROCESSOR}\""
    "PLUGIN_SUFFIX=\"${CMAKE_SHARED_LIBRARY_SUFFIX}\""
    )

target_link_libraries(lyric_common
    PUBLIC
    tempo::tempo_config
    tempo::tempo_tracing
    tempo::tempo_utils
    absl::flat_hash_map
    )

# install targets
install(TARGETS lyric_common EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_common
    )

# add testing subdirectory
add_subdirectory(test)
