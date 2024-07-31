#ifndef LYRIC_PARSER_ARCHETYPE_STATE_ATTR_WRITER_H
#define LYRIC_PARSER_ARCHETYPE_STATE_ATTR_WRITER_H

#include <tempo_utils/attr.h>

namespace lyric_parser {

    class ArchetypeAttr;
    class ArchetypeState;

    class ArchetypeStateAttrWriter : private tempo_utils::AbstractAttrWriterWithState<ArchetypeState> {
    public:
        template<class T>
        static tempo_utils::Result<ArchetypeAttr *> createAttr(
            ArchetypeState *state,
            const tempo_utils::AttrSerde<T> &serde,
            const T &value)
        {
            auto key = serde.getKey();
            ArchetypeStateAttrWriter writer(key, state);
            TU_RETURN_IF_STATUS (serde.writeAttr(&writer, value));
            return writer.getAttr();
        }

        template<class W, class S>
        static tempo_utils::Result<ArchetypeAttr *> createAttr(
            ArchetypeState *state,
            tempo_utils::AttrKey key,
            const tempo_utils::StatefulWritingSerde<W,S> &serde,
            const W &value)
        {
            ArchetypeStateAttrWriter writer(key, state);
            TU_RETURN_IF_STATUS (serde.writeAttr(&writer, value));
            return writer.getAttr();
        }

    private:
        tempo_utils::AttrKey m_key;
        ArchetypeState *m_state;
        ArchetypeAttr *m_attr;

        ArchetypeStateAttrWriter(const tempo_utils::AttrKey &key, ArchetypeState *state);
        ArchetypeAttr *getAttr();
        ArchetypeState *getWriterState() override;
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
        tempo_utils::Result<tu_uint32> putHandle(tempo_utils::AttrHandle handle) override;
    };
}

#endif // LYRIC_PARSER_ARCHETYPE_STATE_ATTR_WRITER_H