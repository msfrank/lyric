
#include <lyric_runtime/serialize_value.h>
#include <lyric_runtime/string_ref.h>
#include <lyric_runtime/url_ref.h>
#include <lyric_serde/patchset_value.h>

tu_uint32
lyric_runtime::serialize_value(const DataCell &value, lyric_serde::PatchsetState &state)
{
    switch (value.type) {

        case DataCellType::NIL: {
            auto appendValueResult = state.appendValue(tempo_schema::AttrValue(nullptr));
            if (appendValueResult.isStatus())
                return INVALID_ADDRESS_U32;
            return appendValueResult.getResult()->getAddress().getAddress();
        }

        case DataCellType::BOOL: {
            auto appendValueResult = state.appendValue(tempo_schema::AttrValue(value.data.b));
            if (appendValueResult.isStatus())
                return INVALID_ADDRESS_U32;
            return appendValueResult.getResult()->getAddress().getAddress();
        }

        case DataCellType::I64: {
            auto appendValueResult = state.appendValue(tempo_schema::AttrValue(value.data.i64));
            if (appendValueResult.isStatus())
                return INVALID_ADDRESS_U32;
            return appendValueResult.getResult()->getAddress().getAddress();
        }

        case DataCellType::DBL: {
            auto appendValueResult = state.appendValue(tempo_schema::AttrValue(value.data.dbl));
            if (appendValueResult.isStatus())
                return INVALID_ADDRESS_U32;
            return appendValueResult.getResult()->getAddress().getAddress();
        }

        case DataCellType::CHAR32: {
            auto appendValueResult = state.appendValue(tempo_schema::AttrValue(static_cast<tu_uint32>(value.data.chr)));
            if (appendValueResult.isStatus())
                return INVALID_ADDRESS_U32;
            return appendValueResult.getResult()->getAddress().getAddress();
        }

        case DataCellType::STRING: {
            std::string utf8;
            if (!value.data.str->utf8Value(utf8))
                return INVALID_ADDRESS_U32;
            auto appendValueResult = state.appendValue(tempo_schema::AttrValue(utf8));
            if (appendValueResult.isStatus())
                return INVALID_ADDRESS_U32;
            return appendValueResult.getResult()->getAddress().getAddress();
        }

        case DataCellType::URL: {
            tempo_utils::Url url;
            if (!value.data.str->uriValue(url))
                return INVALID_ADDRESS_U32;
            auto appendValueResult = state.appendValue(tempo_schema::AttrValue(url.toString()));
            if (appendValueResult.isStatus())
                return INVALID_ADDRESS_U32;
            return appendValueResult.getResult()->getAddress().getAddress();
        }

        case DataCellType::REF: {
            tu_uint32 index;
            if (!value.data.ref->serializeValue(state, index))
                return INVALID_ADDRESS_U32;
            return index;
        }

        default:
            return INVALID_ADDRESS_U32;
    }
}