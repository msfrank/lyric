#ifndef LYRIC_RUNTIME_HASH_DATA_CELL_H
#define LYRIC_RUNTIME_HASH_DATA_CELL_H

#include <absl/hash/hash.h>

#include "data_cell.h"

namespace lyric_runtime {

    template <typename H>
    H AbslHashValue(H h, const DataCell &cell) {
        switch (cell.type) {
            case DataCellType::INVALID:
            case DataCellType::NIL:
            case DataCellType::PRESENT:
                return H::combine(std::move(h), cell.type);
            case DataCellType::BOOL:
                return H::combine(std::move(h), cell.type, cell.data.b);
            case DataCellType::I64:
                return H::combine(std::move(h), cell.type, cell.data.i64);
            case DataCellType::DBL:
                return H::combine(std::move(h), cell.type, cell.data.dbl);
            case DataCellType::CHAR32:
                return H::combine(std::move(h), cell.type, cell.data.chr);
            case DataCellType::UTF8: {
                auto state = H::combine(std::move(h), cell.type);
                return H::combine_contiguous(std::move(state), cell.data.utf8.data, cell.data.utf8.size);
            }
            case DataCellType::REF:
                // NOTE: we hash the pointer value in this case, not the ref contents
                return H::combine(std::move(h), cell.type, cell.data.ref);
                break;
            case DataCellType::CLASS:
            case DataCellType::STRUCT:
            case DataCellType::INSTANCE:
            case DataCellType::CONCEPT:
            case DataCellType::ENUM:
            case DataCellType::FIELD:
            case DataCellType::CALL:
            case DataCellType::ACTION:
            case DataCellType::TYPE:
            case DataCellType::EXISTENTIAL:
            case DataCellType::NAMESPACE:
                return H::combine(std::move(h), cell.type, cell.data.descriptor.assembly, cell.data.descriptor.value);
        }
        TU_UNREACHABLE();
    }
}

#endif // LYRIC_RUNTIME_HASH_DATA_CELL_H
