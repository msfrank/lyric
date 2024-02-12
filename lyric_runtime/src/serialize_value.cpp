
#include <lyric_runtime/serialize_value.h>
#include <lyric_serde/patchset_value.h>

tu_uint32
lyric_runtime::serialize_value(const DataCell &value, lyric_serde::PatchsetState &state)
{
    switch (value.type) {

        case DataCellType::NIL: {
            auto appendValueResult = state.appendValue(tempo_utils::AttrValue(nullptr));
            if (appendValueResult.isStatus())
                return INVALID_ADDRESS_U32;
            return appendValueResult.getResult()->getAddress().getAddress();
        }

        case DataCellType::BOOL: {
            auto appendValueResult = state.appendValue(tempo_utils::AttrValue(value.data.b));
            if (appendValueResult.isStatus())
                return INVALID_ADDRESS_U32;
            return appendValueResult.getResult()->getAddress().getAddress();
        }

        case DataCellType::I64: {
            auto appendValueResult = state.appendValue(tempo_utils::AttrValue(value.data.i64));
            if (appendValueResult.isStatus())
                return INVALID_ADDRESS_U32;
            return appendValueResult.getResult()->getAddress().getAddress();
        }

        case DataCellType::DBL: {
            auto appendValueResult = state.appendValue(tempo_utils::AttrValue(value.data.dbl));
            if (appendValueResult.isStatus())
                return INVALID_ADDRESS_U32;
            return appendValueResult.getResult()->getAddress().getAddress();
        }

        case DataCellType::CHAR32: {
            auto appendValueResult = state.appendValue(tempo_utils::AttrValue(static_cast<tu_uint32>(value.data.chr)));
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