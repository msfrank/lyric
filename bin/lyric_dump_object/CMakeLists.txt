
add_executable(lyric-dump-object src/lyric_dump_object.cpp)

set_target_properties(lyric-dump-object PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${LYRIC_BUILD_BIN_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${BIN_RPATH}
    )

target_link_libraries(lyric-dump-object PRIVATE
    lyric::lyric_object
    tempo::tempo_command
    tempo::tempo_config
    flatbuffers::flatbuffers
    )

# install targets
install(TARGETS lyric-dump-object EXPORT lyric-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
