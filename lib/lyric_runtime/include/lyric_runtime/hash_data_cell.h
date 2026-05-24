#ifndef LYRIC_RUNTIME_HASH_DATA_CELL_H
#define LYRIC_RUNTIME_HASH_DATA_CELL_H

#include <absl/hash/hash.h>

#include "data_cell.h"
#include "descriptor_entry.h"

namespace lyric_runtime {

    template <typename H>
    H AbslHashValue(H h, const DataCell &cell) {
        switch (cell.type) {
            case DataCellType::Invalid:
            case DataCellType::Nil:
            case DataCellType::Undef:
                return H::combine(std::move(h), cell.type);
            case DataCellType::Bool:
                return H::combine(std::move(h), cell.type, cell.data.b);
            case DataCellType::Int64:
                return H::combine(std::move(h), cell.type, cell.data.i64);
            case DataCellType::Float64:
                return H::combine(std::move(h), cell.type, cell.data.dbl);
            case DataCellType::Char32:
                return H::combine(std::move(h), cell.type, cell.data.chr);

            case DataCellType::Bytes:
                // NOTE: we hash the pointer value in this case, not the string content
                return H::combine(std::move(h), cell.type, cell.data.bytes);

            case DataCellType::String:
                // NOTE: we hash the pointer value in this case, not the string content
                return H::combine(std::move(h), cell.type, cell.data.str);

            case DataCellType::Status:
                // NOTE: we hash the pointer value in this case, not the url content
                return H::combine(std::move(h), cell.type, cell.data.status);

            case DataCellType::Namespace:
                // NOTE: we hash the pointer value in this case, not the url content
                return H::combine(std::move(h), cell.type, cell.data.ns);

            case DataCellType::Protocol:
                // NOTE: we hash the pointer value in this case, not the url content
                return H::combine(std::move(h), cell.type, cell.data.protocol);

            case DataCellType::Rest:
                // NOTE: we hash the pointer value in this case, not the url content
                return H::combine(std::move(h), cell.type, cell.data.rest);

            case DataCellType::Ref:
                // NOTE: we hash the pointer value in this case, not the ref content
                return H::combine(std::move(h), cell.type, cell.data.ref);

            case DataCellType::Descriptor:
                // NOTE: we hash the pointer value in this case, not the descriptor content
                return H::combine(std::move(h), cell.type, cell.data.descriptor);

            case DataCellType::Type:
                // NOTE: we hash the pointer value in this case, not the type content
                return H::combine(std::move(h), cell.type, cell.data.type);
        }
        TU_UNREACHABLE();
    }
}

#endif // LYRIC_RUNTIME_HASH_DATA_CELL_H
