
# define global args
ARG lyric_image=lyric:latest

# declare the lyric named context
FROM ${lyric_image} AS lyric

# declare the base stage
FROM scratch

# copy the cache save
COPY --from=lyric /home/jrandomhacker/src/lyric/lyric_cache_save.tgz /home/jrandomhacker/src/lyric/
