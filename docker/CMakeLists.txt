
#######################################################################
#
# declare options
#
#######################################################################

set(DOCKER_PROGRAM ""
    CACHE FILEPATH
    "Path to the docker command. If empty then the program path is auto-detected. Defaults to ''")

set(DOCKER_REQUIRES_SUDO "FALSE"
    CACHE BOOL
    "TRUE if the docker command requires sudo privileges. Defaults to 'FALSE'")

set(DOCKER_PLATFORM_ID ""
    CACHE STRING
    "Target platform for docker image. If empty then the platform is auto-detected. Defaults to ''")

set(DOCKER_BASE_IMAGE "$<IF:$<BOOL:${IS_DEBUG_BUILD}>,timbre:debug-latest,timbre:latest>"
    CACHE STRING
    "The docker image to use as the base. Defaults to 'timbre:debug-latest' for a debug build, otherwise 'timbre:latest'")

set(DOCKER_TEMPO_IMAGE "$<IF:$<BOOL:${IS_DEBUG_BUILD}>,tempo:debug-latest,tempo:latest>"
    CACHE STRING
    "The docker image which contains the Tempo cache save. Defaults to 'tempo:debug-latest' for a debug build, otherwise 'tempo:latest'")

set(DOCKER_REGISTRY ""
    CACHE STRING
    "The docker registry to export to. If empty then defaults to the local image store. Defaults to ''")


#######################################################################
#
# find required and optional dependencies
#
#######################################################################

# find docker CLI
if (DOCKER_PROGRAM)
    set(Docker ${DOCKER_PROGRAM})
    message(STATUS "Using Docker: ${Docker} (specified by DOCKER_PROGRAM)")
else()
    find_program(Docker docker)
    if (Docker EQUAL "Docker-NOTFOUND")
        message(STATUS "Docker not found, skipping generation of docker targets")
    else()
        message(STATUS "Found Docker: ${Docker}")
    endif()
endif()


#######################################################################
#
# configure tools and paths
#
#######################################################################

if (DOCKER_REQUIRES_SUDO)
    set(DOCKER "sudo;${Docker}")
else ()
    set(DOCKER ${Docker})
endif ()


#######################################################################
#
# define docker build targets
#
#######################################################################

if (DOCKER_REGISTRY)
    set(DOCKER_OUTPUT "type=registry,push=true")
else()
    set(DOCKER_OUTPUT "type=docker")
endif()

# set the PLATFORM_ID
if (DOCKER_PLATFORM_ID)
    set(PLATFORM_ID ${DOCKER_PLATFORM_ID})
else()
    set(PLATFORM_ID "linux/${CMAKE_SYSTEM_PROCESSOR}")
endif()
if (NOT PLATFORM_ID)
    message(FATAL_ERROR "failed to determine docker platform id")
endif()

set(DOCKERFILE_PATH "${CMAKE_SOURCE_DIR}/docker/Dockerfile")

# build the lyric image
add_custom_target(build_docker_image
    COMMAND ${DOCKER} build
    --platform ${PLATFORM_ID}
    --build-arg base_image=${DOCKER_BASE_IMAGE}
    --build-arg tempo_image=${DOCKER_TEMPO_IMAGE}
    -o ${DOCKER_OUTPUT}
    -t "${DOCKER_REGISTRY}lyric"
    -t "${DOCKER_REGISTRY}lyric:${CMAKE_PROJECT_VERSION}"
    -f ${DOCKERFILE_PATH}
    --label "org.opencontainers.image.version=${CMAKE_PROJECT_VERSION}"
    --label "org.opencontainers.image.source=${CMAKE_PROJECT_HOMEPAGE_URL}"
    --label "org.opencontainers.image.description=${CMAKE_PROJECT_DESCRIPTION}"
    --label "org.opencontainers.image.licenses=${LYRIC_PROJECT_LICENSE}"
    .
    COMMENT "building lyric docker image"
    DEPENDS ${DOCKERFILE_PATH}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
