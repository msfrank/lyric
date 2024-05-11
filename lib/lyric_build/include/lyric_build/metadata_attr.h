#ifndef LYRIC_BUILD_METADATA_ATTR_H
#define LYRIC_BUILD_METADATA_ATTR_H

#include <lyric_build/metadata_state.h>
#include <tempo_utils/attr.h>

namespace lyric_build {

    class MetadataAttr {

    public:
        MetadataAttr(
            AttrId id,
            tempo_utils::AttrValue value,
            AttrAddress address,
            MetadataState *state);

        AttrId getAttrId() const;
        tempo_utils::AttrValue getAttrValue() const;
        AttrAddress getAddress() const;

    private:
        AttrId m_id;
        tempo_utils::AttrValue m_value;
        AttrAddress m_address;
        MetadataState *m_state;
    };
}

#endif // LYRIC_BUILD_METADATA_ATTR_H