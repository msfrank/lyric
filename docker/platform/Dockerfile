
# define global args
ARG base_image=timbre:latest
ARG tempo_image=tempo:latest

# declare the tempo named context
FROM ${tempo_image} AS tempo

# declare the base stage
FROM ${base_image} AS base

# define stage args
ARG is_debug_build=0

# switch current user
USER jrandomhacker

# copy the tempo cache save from the tempo image
COPY --from=tempo /home/jrandomhacker/src/tempo/tempo_cache_save.tgz /tmp/

# load the tempo package into the conan cache
RUN conan cache restore /tmp/tempo_cache_save.tgz

# copy lyric repository into the image
ADD --chown=jrandomhacker . /home/jrandomhacker/src/lyric

# change to lyric repository directory
WORKDIR /home/jrandomhacker/src/lyric

# build lyric in the conan cache
RUN conan create .

# create a conan cache save containing only lyric
RUN conan cache save "lyric/*:*" --file lyric_cache_save.tgz

# remove lyric temporary files
RUN conan cache clean --download --temp --backup-sources 'lyric/*'

# if this is not a debug build then remove lyric build files
ENV IS_DEBUG_BUILD=$is_debug_build
RUN if test $IS_DEBUG_BUILD == 0; then conan cache clean --source --build 'lyric/*'; fi
