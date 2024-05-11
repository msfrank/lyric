//#ifndef LYRIC_SERDE_PATCHSET_VALUE_PARSER_H
//#define LYRIC_SERDE_PATCHSET_VALUE_PARSER_H
//
//#include <lyric_serde/serde_types.h>
//#include <tempo_utils/attr.h>
//
//namespace lyric_serde {
//
//    class PatchsetValueParser : public tempo_utils::AbstractAttrParser {
//    public:
//        PatchsetValueParser(std::shared_ptr<const internal::PatchsetReader> reader);
//        virtual tempo_utils::AttrStatus getNil(tu_uint32 index, std::nullptr_t &nil) override;
//        virtual tempo_utils::AttrStatus getBool(tu_uint32 index, bool &b) override;
//        virtual tempo_utils::AttrStatus getInt64(tu_uint32 index, tu_int64 &i64) override;
//        virtual tempo_utils::AttrStatus getFloat64(tu_uint32 index, double &dbl) override;
//        virtual tempo_utils::AttrStatus getUInt64(tu_uint32 index, tu_uint64 &u64) override;
//        virtual tempo_utils::AttrStatus getUInt32(tu_uint32 index, tu_uint32 &u32) override;
//        virtual tempo_utils::AttrStatus getUInt16(tu_uint32 index, tu_uint16 &u16) override;
//        virtual tempo_utils::AttrStatus getUInt8(tu_uint32 index, tu_uint8 &u8) override;
//        virtual tempo_utils::AttrStatus getString(tu_uint32 index, std::string &str) override;
//
//    private:
//        std::shared_ptr<const internal::PatchsetReader> m_reader;
//    };
//}
//
//#endif // LYRIC_SERDE_PATCHSET_VALUE_PARSER_H