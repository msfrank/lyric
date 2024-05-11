//#ifndef LYRIC_SERDE_PATCHSET_VALUE_WRITER_H
//#define LYRIC_SERDE_PATCHSET_VALUE_WRITER_H
//
//#include <tempo_utils/attr.h>
//
//namespace lyric_serde {
//
//    class PatchsetState;
//
//    class PatchsetValueWriter : public tempo_utils::AbstractAttrWriter {
//    public:
//        PatchsetValueWriter(const tempo_utils::AttrKey &key, PatchsetState *state);
//        tempo_utils::AttrResult<tu_uint32> putNamespace(const tempo_utils::Url &nsUrl) override;
//        tempo_utils::AttrResult<tu_uint32> putNil() override;
//        tempo_utils::AttrResult<tu_uint32> putBool(bool b) override;
//        tempo_utils::AttrResult<tu_uint32> putInt64(tu_int64 i64) override;
//        tempo_utils::AttrResult<tu_uint32> putFloat64(double dbl) override;
//        tempo_utils::AttrResult<tu_uint32> putUInt64(tu_uint64 u64) override;
//        tempo_utils::AttrResult<tu_uint32> putUInt32(tu_uint32 u32) override;
//        tempo_utils::AttrResult<tu_uint32> putUInt16(tu_uint16 u16) override;
//        tempo_utils::AttrResult<tu_uint32> putUInt8(tu_uint8 u8) override;
//        tempo_utils::AttrResult<tu_uint32> putString(std::string_view str) override;
//
//    private:
//        tempo_utils::AttrKey m_key;
//        PatchsetState *m_state;
//    };
//}
//
//#endif // LYRIC_SERDE_PATCHSET_VALUE_WRITER_H