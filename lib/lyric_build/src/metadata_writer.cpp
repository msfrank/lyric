
#include <lyric_build/metadata_writer.h>

#include "lyric_build/build_result.h"

lyric_build::MetadataWriter::MetadataWriter(const MetadataWriterOptions &options)
    : m_options(options)
{
}

tempo_utils::Status
lyric_build::MetadataWriter::configure()
{
    if (m_state != nullptr)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "metadata writer is already configured");
    auto state = std::make_unique<MetadataState>();

    if (m_options.metadata.isValid()) {
        TU_RETURN_IF_NOT_OK (state->load(m_options.metadata));
    }

    m_state = std::move(state);
    return {};
}

tempo_utils::Result<lyric_build::LyricMetadata>
lyric_build::MetadataWriter::toMetadata()
{
    if (m_state == nullptr)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "metadata writer is not configured");
    return m_state->toMetadata();
}