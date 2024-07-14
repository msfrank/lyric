#ifndef LYRIC_PARSER_ARCHETYPE_ATTR_PARSER_H
#define LYRIC_PARSER_ARCHETYPE_ATTR_PARSER_H

#include <tempo_utils/attr.h>

#include "parser_types.h"

namespace lyric_parser {

    class LyricArchetype;

    class ArchetypeAttrParser : public tempo_utils::AbstractAttrParserWithState<
        std::shared_ptr<const internal::ArchetypeReader>> {

    public:
        explicit ArchetypeAttrParser(std::shared_ptr<const internal::ArchetypeReader> reader);
        std::shared_ptr<const internal::ArchetypeReader> *getParserState() override;
        tempo_utils::Status getNil(tu_uint32 index, std::nullptr_t &nil) override;
        tempo_utils::Status getBool(tu_uint32 index, bool &b) override;
        tempo_utils::Status getInt64(tu_uint32 index, tu_int64 &i64) override;
        tempo_utils::Status getFloat64(tu_uint32 index, double &dbl) override;
        tempo_utils::Status getUInt64(tu_uint32 index, tu_uint64 &u64) override;
        tempo_utils::Status getUInt32(tu_uint32 index, tu_uint32 &u32) override;
        tempo_utils::Status getUInt16(tu_uint32 index, tu_uint16 &u16) override;
        tempo_utils::Status getUInt8(tu_uint32 index, tu_uint8 &u8) override;
        tempo_utils::Status getString(tu_uint32 index, std::string &str) override;

    private:
        std::shared_ptr<const internal::ArchetypeReader> m_reader;
    };
}

#endif // LYRIC_PARSER_ARCHETYPE_ATTR_PARSER_H