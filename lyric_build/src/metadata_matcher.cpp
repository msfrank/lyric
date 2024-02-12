
#include <absl/container/flat_hash_map.h>

#include <lyric_build/generated/metadata.h>
#include <lyric_build/internal/metadata_reader.h>
#include <lyric_build/metadata_matcher.h>

inline bool
value_matches_filter(
    const lyric_build::LyricMetadata &metadata,
    const lbm1::AttrDescriptor *metadataAttr,
    const lyric_build::LyricMetadata &filter,
    const lbm1::AttrDescriptor *filterAttr)
{
    if (metadataAttr->attr_value_type() != filterAttr->attr_value_type())
        return false;
    switch (metadataAttr->attr_value_type()) {
        case lbm1::Value::TrueFalseNilValue:
            return metadataAttr->attr_value_as_TrueFalseNilValue()->tfn()
                == filterAttr->attr_value_as_TrueFalseNilValue()->tfn();
        case lbm1::Value::Int64Value:
            return metadataAttr->attr_value_as_Int64Value()->i64()
                == filterAttr->attr_value_as_Int64Value()->i64();
        case lbm1::Value::Float64Value:
            return metadataAttr->attr_value_as_Float64Value()->f64()
                == filterAttr->attr_value_as_Float64Value()->f64();
        case lbm1::Value::UInt64Value:
            return metadataAttr->attr_value_as_UInt64Value()->u64()
                   == filterAttr->attr_value_as_UInt64Value()->u64();
        case lbm1::Value::UInt32Value:
            return metadataAttr->attr_value_as_UInt32Value()->u32()
                == filterAttr->attr_value_as_UInt32Value()->u32();
        case lbm1::Value::UInt16Value:
            return metadataAttr->attr_value_as_UInt16Value()->u16()
                == filterAttr->attr_value_as_UInt16Value()->u16();
        case lbm1::Value::UInt8Value:
            return metadataAttr->attr_value_as_UInt8Value()->u8()
                == filterAttr->attr_value_as_UInt8Value()->u8();
        case lbm1::Value::StringValue:
            return metadataAttr->attr_value_as_StringValue()->utf8()->string_view()
                == filterAttr->attr_value_as_StringValue()->utf8()->string_view();
        default:
            return false;
    }
}

bool
lyric_build::metadata_matches_all_filters(const LyricMetadata &metadata, const LyricMetadata &filters)
{
    auto metadataReader = metadata.getReader();

    absl::flat_hash_map<std::string,tu_uint32> metadataNamespaces;
    for (tu_uint32 i = 0; i < metadataReader->numNamespaces(); i++) {
        const auto *metadataNs = metadataReader->getNamespace(i);
        metadataNamespaces[metadataNs->ns_url()->str()] = i;
    }

    absl::flat_hash_map<std::pair<uint32_t,uint32_t>,const lbm1::AttrDescriptor *> metadataAttrs;
    for (tu_uint32 i = 0; i < metadataReader->numAttrs(); i++) {
        const auto *metadataAttr = metadataReader->getAttr(i);
        std::pair<uint32_t,uint32_t> key(metadataAttr->attr_ns(), metadataAttr->attr_id());
        metadataAttrs[key] = metadataAttr;
    }

    auto filterReader = filters.getReader();

    std::vector<tu_uint32> filterNamespaces(filterReader->numNamespaces());
    for (tu_uint32 i = 0; i < filterReader->numNamespaces(); i++) {
        const auto *filterNs = filterReader->getNamespace(i);
        auto nsUrl = filterNs->ns_url()->str();
        if (!metadataNamespaces.contains(nsUrl))
            return false;
        filterNamespaces[i] = metadataNamespaces.at(nsUrl);
    }

    for (uint32_t i = 0; i < filterReader->numAttrs(); i++) {
        const auto *filterAttr = filterReader->getAttr(i);
        if (filterNamespaces.size() <= filterAttr->attr_ns())
            return false;
        auto metadataNs = filterNamespaces[filterAttr->attr_ns()];

        std::pair<uint32_t,uint32_t> key(metadataNs, filterAttr->attr_id());
        if (!metadataAttrs.contains(key))
            return false;

        const auto *metadataAttr = metadataAttrs[key];
        if (!value_matches_filter(metadata, metadataAttr, filters, filterAttr))
            return false;
    }

    return true;
}
