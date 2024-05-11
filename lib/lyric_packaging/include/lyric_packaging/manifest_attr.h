#ifndef LYRIC_PACKAGING_MANIFEST_ATTR_H
#define LYRIC_PACKAGING_MANIFEST_ATTR_H

#include <tempo_utils/attr.h>

#include "manifest_state.h"

namespace lyric_packaging {

    class ManifestAttr {

    public:
        ManifestAttr(
            AttrId id,
            tempo_utils::AttrValue value,
            AttrAddress address,
            ManifestState *state);

        AttrId getAttrId() const;
        tempo_utils::AttrValue getAttrValue() const;
        AttrAddress getAddress() const;

    private:
        AttrId m_id;
        tempo_utils::AttrValue m_value;
        AttrAddress m_address;
        ManifestState *m_state;
    };
}

#endif // LYRIC_PACKAGING_MANIFEST_ATTR_H