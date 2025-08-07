
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_typing as a shared library
add_library(lyric_typing SHARED)
add_library(lyric::lyric_typing ALIAS lyric_typing)

set(LYRIC_TYPING_INCLUDES
    include/lyric_typing/callsite_reifier.h
    include/lyric_typing/compare_assignable.h
    include/lyric_typing/member_reifier.h
    include/lyric_typing/parse_assignable.h
    include/lyric_typing/parse_pack.h
    include/lyric_typing/resolve_assignable.h
    include/lyric_typing/resolve_pack.h
    include/lyric_typing/resolve_template.h
    include/lyric_typing/type_spec.h
    include/lyric_typing/type_system.h
    include/lyric_typing/typing_result.h
    include/lyric_typing/typing_types.h
    include/lyric_typing/unify_assignable.h
    )
set_target_properties(lyric_typing PROPERTIES PUBLIC_HEADER "${LYRIC_TYPING_INCLUDES}")

target_sources(lyric_typing PRIVATE
    src/callsite_reifier.cpp
    src/compare_assignable.cpp
    src/member_reifier.cpp
    src/parse_assignable.cpp
    src/parse_pack.cpp
    src/resolve_assignable.cpp
    src/resolve_pack.cpp
    src/resolve_template.cpp
    src/type_spec.cpp
    src/type_system.cpp
    src/typing_result.cpp
    src/typing_types.cpp
    src/unify_assignable.cpp
    )

# set the library version
set_target_properties(lyric_typing PROPERTIES VERSION "${PROJECT_VERSION}" SOVERSION "${PROJECT_VERSION_MAJOR}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_typing PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_typing PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_typing PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(lyric_typing
    PUBLIC
    lyric::lyric_assembler
    lyric::lyric_object
    lyric::lyric_parser
    tempo::tempo_tracing
    tempo::tempo_utils
    absl::flat_hash_map
    absl::flat_hash_set
    PRIVATE
    absl::strings
    )

# install targets
install(TARGETS lyric_typing EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_typing
    )

# add testing subdirectory
add_subdirectory(test)
