#ifndef LYRIC_SERDE_PATCHSET_VALUE_H
#define LYRIC_SERDE_PATCHSET_VALUE_H

#include <lyric_serde/serde_types.h>
#include <tempo_schema/attr.h>

#include "patchset_state.h"

namespace lyric_serde {

    enum class VariantType {
        Invalid,
        Intrinsic,
        Attr,
        Element,
    };

    struct AttrValue {
        tu_int16 ns;
        tu_uint32 id;
        ValueAddress value;
    };

    struct ElementValue {
        tu_int16 ns;
        tu_uint32 id;
        std::vector<ValueAddress> children;
    };

    struct ValueVariant {
        VariantType type;
        tempo_schema::AttrValue intrinsic;
        AttrValue attr;
        ElementValue element;
    };

    class PatchsetValue {

    public:
        PatchsetValue(
            std::unique_ptr<ValueVariant> &&value,
            ValueAddress address,
            PatchsetState *state);

        const ValueVariant *getValue() const;
        ValueAddress getAddress() const;

    private:
        std::unique_ptr<ValueVariant> m_variant;
        ValueAddress m_address;
        PatchsetState *m_state;
    };
}

#endif // LYRIC_SERDE_PATCHSET_VALUE_H