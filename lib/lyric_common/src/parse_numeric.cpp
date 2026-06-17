
#include <absl/strings/charconv.h>
#include <boost/multiprecision/cpp_int.hpp>

#include <lyric_common/parse_numeric.h>

#include "lyric_common/common_status.h"

class BaseAccumulator {
public:
    explicit BaseAccumulator(tu_uint8 base, bool negative)
        : m_base(base),
          m_negative(negative)
    {
    }
    virtual ~BaseAccumulator() = default;

    virtual tempo_utils::Status parseNextDigit(char digit) = 0;

protected:
    tempo_utils::Status applyDigit(tu_uint8 value)
    {
        boost::multiprecision::cpp_int acc = value * (boost::multiprecision::pow(m_base, m_digit));
        TU_LOG_INFO << "acc=" << to_string(acc)
                    << " base=" << to_string(m_base)
                    << " digit=" << m_digit;
        m_total += acc;
        TU_LOG_INFO << "total=" << to_string(m_total);
        m_digit++;
        return {};
    }

    boost::multiprecision::cpp_int getTotal()
    {
        return m_negative? -m_total : m_total;
    }

private:
    boost::multiprecision::cpp_int m_base;
    bool m_negative;
    tu_uint32 m_digit = 0;
    boost::multiprecision::cpp_int m_total = 0;
};

template<class SizedType>
class SizedAccumulator : public BaseAccumulator {
public:
    SizedAccumulator(tu_uint8 base, bool negative) : BaseAccumulator(base, negative) {}

    bool getTotal(SizedType &total) {
        boost::multiprecision::cpp_int t = BaseAccumulator::getTotal();
        if (t < std::numeric_limits<SizedType>::min() || t > std::numeric_limits<SizedType>::max())
            return false;
        total = t.convert_to<SizedType>();
        return true;
    }
};

template<class SizedType>
class DecimalAccumulator : public SizedAccumulator<SizedType> {
public:
    explicit DecimalAccumulator(bool negative) : SizedAccumulator<SizedType>(10, negative) {}

    tempo_utils::Status parseNextDigit(char digit) override
    {
        tu_uint8 value;
        switch (digit) {
            case '0': value = 0; break;
            case '1': value = 1; break;
            case '2': value = 2; break;
            case '3': value = 3; break;
            case '4': value = 4; break;
            case '5': value = 5; break;
            case '6': value = 6; break;
            case '7': value = 7; break;
            case '8': value = 8; break;
            case '9': value = 9; break;
            default:
                return lyric_common::CommonStatus::forCondition(lyric_common::CommonCondition::kCommonInvariant,
                    "invalid decimal digit {}", digit);
        }
        return BaseAccumulator::applyDigit(value);
    }
};

template<class SizedType>
class HexAccumulator : public SizedAccumulator<SizedType> {
public:
    explicit HexAccumulator(bool negative) : SizedAccumulator<SizedType>(16, negative) {}

    tempo_utils::Status parseNextDigit(char digit) override
    {
        tu_uint8 value;
        switch (digit) {
            case '0': value = 0; break;
            case '1': value = 1; break;
            case '2': value = 2; break;
            case '3': value = 3; break;
            case '4': value = 4; break;
            case '5': value = 5; break;
            case '6': value = 6; break;
            case '7': value = 7; break;
            case '8': value = 8; break;
            case '9': value = 9; break;
            case 'A': value = 10; break;
            case 'B': value = 11; break;
            case 'C': value = 12; break;
            case 'D': value = 13; break;
            case 'E': value = 14; break;
            case 'F': value = 15; break;
            default:
                return lyric_common::CommonStatus::forCondition(lyric_common::CommonCondition::kCommonInvariant,
                    "invalid hexadecimal digit {}", digit);
        }
        return BaseAccumulator::applyDigit(value);
    }
};

template<class SizedType>
class OctalAccumulator : public SizedAccumulator<SizedType> {
public:
    explicit OctalAccumulator(bool negative) : SizedAccumulator<SizedType>(8, negative) {}

    tempo_utils::Status parseNextDigit(char digit) override
    {
        tu_uint8 value;
        switch (digit) {
            case '0': value = 0; break;
            case '1': value = 1; break;
            case '2': value = 2; break;
            case '3': value = 3; break;
            case '4': value = 4; break;
            case '5': value = 5; break;
            case '6': value = 6; break;
            case '7': value = 7; break;
            default:
                return lyric_common::CommonStatus::forCondition(lyric_common::CommonCondition::kCommonInvariant,
                    "invalid octal digit {}", digit);
        }
        return BaseAccumulator::applyDigit(value);
    }
};

template<class SizedType>
class BinaryAccumulator : public SizedAccumulator<SizedType> {
public:
    explicit BinaryAccumulator(bool negative) : SizedAccumulator<SizedType>(2, negative) {}

    tempo_utils::Status parseNextDigit(char digit) override
    {
        tu_uint8 value;
        switch (digit) {
            case '0': value = 0; break;
            case '1': value = 1; break;
            default:
                return lyric_common::CommonStatus::forCondition(lyric_common::CommonCondition::kCommonInvariant,
                    "invalid binary digit {}", digit);
        }
        return BaseAccumulator::applyDigit(value);
    }
};

template<class SizedType>
tempo_utils::Status
make_accumulator(
    std::string_view &s,
    lyric_common::NumericBase base,
    std::unique_ptr<SizedAccumulator<SizedType>> &acc)
{
    if (s.empty())
        return lyric_common::CommonStatus::forCondition(
            lyric_common::CommonCondition::kCommonInvariant, "invalid numeric literal ''");

    bool negative = false;
    if (s.starts_with("-")) {
        negative = true;
        s = s.substr(1);
    }

    if (base == lyric_common::NumericBase::Autodetect) {
        if (s.starts_with("0x")) {
            s = s.substr(2);
            acc = std::make_unique<HexAccumulator<SizedType>>(negative);
            return {};
        }
        if (s.starts_with("0o")) {
            s = s.substr(2);
            acc = std::make_unique<OctalAccumulator<SizedType>>(negative);
            return {};
        }
        if (s.starts_with("0b")) {
            s = s.substr(2);
            acc = std::make_unique<BinaryAccumulator<SizedType>>(negative);
            return {};
        }
        acc = std::make_unique<DecimalAccumulator<SizedType>>(negative);
        return {};
    }

    switch (base) {
        case lyric_common::NumericBase::Decimal:
            acc = std::make_unique<DecimalAccumulator<SizedType>>(negative);
            return {};
        case lyric_common::NumericBase::Hex:
            acc = std::make_unique<HexAccumulator<SizedType>>(negative);
            return {};
        case lyric_common::NumericBase::Octal:
            acc = std::make_unique<OctalAccumulator<SizedType>>(negative);
            return {};
        case lyric_common::NumericBase::Binary:
            acc = std::make_unique<BinaryAccumulator<SizedType>>(negative);
            return {};
        default:
            return lyric_common::CommonStatus::forCondition(
                lyric_common::CommonCondition::kCommonInvariant, "invalid numeric base for literal '{}'", s);
    }
}

template<class SizedType>
tempo_utils::Result<SizedType>
parse_integral(std::string_view s, lyric_common::NumericBase base)
{
    std::unique_ptr<SizedAccumulator<SizedType>> acc;
    TU_RETURN_IF_NOT_OK (make_accumulator(s, base, acc));
    for (auto it = s.rbegin(); it != s.rend(); it++) {
        char digit = *it;
        if (digit == '_')
            continue;
        TU_RETURN_IF_NOT_OK (acc->parseNextDigit(digit));
    }
    SizedType total;
    if (!acc->getTotal(total))
        return lyric_common::CommonStatus::forCondition(
            lyric_common::CommonCondition::kCommonInvariant, "numeric literal '{}' is out of range", s);
    return total;
}

tempo_utils::Result<tu_int64>
lyric_common::parse_I64(std::string_view s, NumericBase base)
{
    return parse_integral<tu_int64>(s, base);
}

tempo_utils::Result<tu_int32>
lyric_common::parse_I32(std::string_view s, NumericBase base)
{
    return parse_integral<tu_int32>(s, base);
}

tempo_utils::Result<tu_int16>
lyric_common::parse_I16(std::string_view s, NumericBase base)
{
    return parse_integral<tu_int16>(s, base);
}

tempo_utils::Result<tu_int8>
lyric_common::parse_I8(std::string_view s, NumericBase base)
{
    return parse_integral<tu_int8>(s, base);
}

tempo_utils::Result<tu_uint64>
lyric_common::parse_U64(std::string_view s, NumericBase base)
{
    return parse_integral<tu_uint64>(s, base);
}

tempo_utils::Result<tu_uint32>
lyric_common::parse_U32(std::string_view s, NumericBase base)
{
    return parse_integral<tu_uint32>(s, base);
}

tempo_utils::Result<tu_uint16>
lyric_common::parse_U16(std::string_view s, NumericBase base)
{
    return parse_integral<tu_uint16>(s, base);
}

tempo_utils::Result<tu_uint8>
lyric_common::parse_U8(std::string_view s, NumericBase base)
{
    return parse_integral<tu_uint8>(s, base);
}

template<class SizedType>
tempo_utils::Result<SizedType>
parse_floating_point(std::string_view s, lyric_common::NumericBase base, bool scientific)
{
    if (s.empty())
        return lyric_common::CommonStatus::forCondition(
            lyric_common::CommonCondition::kCommonInvariant, "invalid numeric literal ''");

    // we preserve s and use chars as a mutable view
    std::string_view chars = s;

    // if '+' prefix is present then strip it before processing
    if (chars.front() == '+') {
        if (chars.size() == 1)
            return lyric_common::CommonStatus::forCondition(
                lyric_common::CommonCondition::kCommonInvariant, "invalid numeric literal '{}'", s);
        chars = chars.substr(1);
    }

    absl::chars_format fmt = absl::chars_format::fixed;

    // set the base, performing autodetection if needed
    switch (base) {
        case lyric_common::NumericBase::Autodetect: {
            auto detection = chars.front() == '-'? chars.substr(1) : chars;
            if (detection.starts_with("0x")) {
                fmt |= absl::chars_format::hex;
            }
            // we don't support octal or binary
            if (detection.starts_with("0o"))
                return lyric_common::CommonStatus::forCondition(lyric_common::CommonCondition::kCommonInvariant,
                    "invalid numeric base for literal '{}'", s);
            if (detection.starts_with("0b"))
                return lyric_common::CommonStatus::forCondition(lyric_common::CommonCondition::kCommonInvariant,
                    "invalid numeric base for literal '{}'", s);
            // we default to decimal if no prefix was found
            break;
        }
        case lyric_common::NumericBase::Decimal:
            break;
        case lyric_common::NumericBase::Hex:
            fmt |= absl::chars_format::hex;
            break;
        default:
            return lyric_common::CommonStatus::forCondition(lyric_common::CommonCondition::kCommonInvariant,
                "invalid numeric base for literal '{}'", s);
    }
    if (scientific) {
        fmt |= absl::chars_format::scientific;
    }

    const char *start = chars.data();
    const char *end = start + chars.size();

    SizedType conv;
    auto result = absl::from_chars(start, end, conv, fmt);
    if (result.ptr == end)
        return conv;

    switch (result.ec) {
        case std::errc::result_out_of_range:
            return lyric_common::CommonStatus::forCondition(lyric_common::CommonCondition::kCommonInvariant,
                "numeric literal '{}' is out of range", s);
        case std::errc::invalid_argument:
        default:
            return lyric_common::CommonStatus::forCondition(lyric_common::CommonCondition::kCommonInvariant,
                "invalid numeric literal '{}'", s);
    }
}

tempo_utils::Result<float>
lyric_common::parse_F32(std::string_view s, NumericBase base, bool scientific)
{
    return parse_floating_point<float>(s, base, scientific);
}

tempo_utils::Result<double>
lyric_common::parse_F64(std::string_view s, NumericBase base, bool scientific)
{
    return parse_floating_point<double>(s, base, scientific);
}
