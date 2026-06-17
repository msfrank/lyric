#ifndef LYRIC_COMMON_PARSE_NUMERIC_H
#define LYRIC_COMMON_PARSE_NUMERIC_H

#include <tempo_utils/result.h>

namespace lyric_common {

    enum class NumericBase {
        Invalid,
        Autodetect,
        Decimal,
        Hex,
        Octal,
        Binary,
    };

    tempo_utils::Result<tu_int64> parse_I64(std::string_view s, NumericBase base = NumericBase::Autodetect);
    tempo_utils::Result<tu_int32> parse_I32(std::string_view s, NumericBase base = NumericBase::Autodetect);
    tempo_utils::Result<tu_int16> parse_I16(std::string_view s, NumericBase base = NumericBase::Autodetect);
    tempo_utils::Result<tu_int8> parse_I8(std::string_view s, NumericBase base = NumericBase::Autodetect);

    tempo_utils::Result<tu_uint64> parse_U64(std::string_view s, NumericBase base = NumericBase::Autodetect);
    tempo_utils::Result<tu_uint32> parse_U32(std::string_view s, NumericBase base = NumericBase::Autodetect);
    tempo_utils::Result<tu_uint16> parse_U16(std::string_view s, NumericBase base = NumericBase::Autodetect);
    tempo_utils::Result<tu_uint8> parse_U8(std::string_view s, NumericBase base = NumericBase::Autodetect);

    tempo_utils::Result<float> parse_F32(std::string_view s, NumericBase base = NumericBase::Autodetect, bool scientific = false);
    tempo_utils::Result<double> parse_F64(std::string_view s, NumericBase base = NumericBase::Autodetect, bool scientific = false);
}

#endif // LYRIC_COMMON_PARSE_NUMERIC_H
