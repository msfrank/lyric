#ifndef LYRIC_BUILD_METADATA_ATTR_WRITER_H
#define LYRIC_BUILD_METADATA_ATTR_WRITER_H

#include <lyric_build/metadata_state.h>
#include <tempo_schema/abstract_attr_writer.h>

namespace lyric_build {

    class MetadataAttrWriter : public tempo_schema::AbstractAttrWriter {
    public:
        MetadataAttrWriter(const tempo_schema::AttrKey &key, MetadataState *state);
        tempo_utils::Result<tu_uint32> putNamespace(const tempo_utils::Url &nsUrl) override;
        tempo_utils::Result<tu_uint32> putNil() override;
        tempo_utils::Result<tu_uint32> putBool(bool b) override;
        tempo_utils::Result<tu_uint32> putInt64(tu_int64 i64) override;
        tempo_utils::Result<tu_uint32> putFloat64(double dbl) override;
        tempo_utils::Result<tu_uint32> putUInt64(tu_uint64 u64) override;
        tempo_utils::Result<tu_uint32> putUInt32(tu_uint32 u32) override;
        tempo_utils::Result<tu_uint32> putUInt16(tu_uint16 u16) override;
        tempo_utils::Result<tu_uint32> putUInt8(tu_uint8 u8) override;
        tempo_utils::Result<tu_uint32> putString(std::string_view str) override;

    private:
        tempo_schema::AttrKey m_key;
        MetadataState *m_state;
    };
}

#endif // LYRIC_BUILD_METADATA_ATTR_WRITER_H