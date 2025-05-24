
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_schema as a shared library
add_library(lyric_schema INTERFACE)
add_library(lyric::lyric_schema ALIAS lyric_schema)

set(LYRIC_SCHEMA_INCLUDES
    include/lyric_schema/assembler_schema.h
    include/lyric_schema/ast_schema.h
    include/lyric_schema/build_schema.h
    include/lyric_schema/compiler_schema.h
    include/lyric_schema/dcmi_elements_schema.h
    include/lyric_schema/literal_schema.h
    include/lyric_schema/parser_schema.h
    include/lyric_schema/posix_stat_schema.h
    )
set_target_properties(lyric_schema PROPERTIES PUBLIC_HEADER "${LYRIC_SCHEMA_INCLUDES}")

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_schema INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(lyric_schema INTERFACE tempo::tempo_utils)

# install targets
install(TARGETS lyric_schema EXPORT lyric-targets
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_schema
    )

# add testing subdirectory
#add_subdirectory(test)
