
#include <lyric_build/metadata_writer.h>

lyric_build::MetadataWriter::MetadataWriter()
    : MetadataWriter(MetadataWriterOptions{})
{
}

lyric_build::MetadataWriter::MetadataWriter(const MetadataWriterOptions &options)
    : m_options(options),
      m_state()
{
}

tempo_utils::Result<lyric_build::LyricMetadata>
lyric_build::MetadataWriter::toMetadata()
{
    return m_state.toMetadata();
}