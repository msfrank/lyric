#ifndef LYRIC_RUNTIME_DATA_CELL_H
#define LYRIC_RUNTIME_DATA_CELL_H

#include <unicode/ustring.h>

#include <tempo_utils/log_message.h>

#include "runtime_types.h"
#include "literal_cell.h"

namespace lyric_runtime {

    // forward declarations
    class BaseRef;

    enum class DataCellType : uint8_t {
        INVALID,
        NIL,
        PRESENT,
        BOOL,
        I64,
        DBL,
        CHAR32,
        UTF8,
        REF,
        CLASS,
        STRUCT,
        INSTANCE,
        CONCEPT,
        ENUM,
        CALL,
        FIELD,
        ACTION,
        TYPE,
        EXISTENTIAL,
        NAMESPACE,
    };

    struct DataCell {

        DataCell();
        explicit DataCell(bool b);
        explicit DataCell(tu_int64 i64);
        explicit DataCell(double dbl);
        explicit DataCell(UChar32 char32);
        DataCell(const DataCell &other);
        DataCell(DataCell &&other) noexcept;
        ~DataCell() = default;

        DataCell& operator=(const DataCell &other);
        DataCell& operator=(DataCell &&other) noexcept;

        bool isValid() const;
        std::string toString() const;

        static DataCell nil();
        static DataCell present();
        static DataCell forLiteral(const lyric_runtime::LiteralCell &literal);
        static DataCell forUtf8(const char *data, tu_int32 size);
        static DataCell forRef(BaseRef *ref);
        static DataCell forClass(tu_uint32 assemblyIndex, tu_uint32 classIndex);
        static DataCell forStruct(tu_uint32 assemblyIndex, tu_uint32 structIndex);
        static DataCell forInstance(tu_uint32 assemblyIndex, tu_uint32 instanceIndex);
        static DataCell forConcept(tu_uint32 assemblyIndex, tu_uint32 conceptIndex);
        static DataCell forEnum(tu_uint32 assemblyIndex, tu_uint32 enumIndex);
        static DataCell forCall(tu_uint32 assemblyIndex, tu_uint32 callIndex);
        static DataCell forField(tu_uint32 assemblyIndex, tu_uint32 fieldIndex);
        static DataCell forAction(tu_uint32 assemblyIndex, tu_uint32 actionIndex);
        static DataCell forType(tu_uint32 assemblyIndex, tu_uint32 typeIndex);
        static DataCell forExistential(tu_uint32 assemblyIndex, tu_uint32 existentialIndex);
        static DataCell forNamespace(tu_uint32 assemblyIndex, tu_uint32 namespaceIndex);

        DataCellType type;
        union {
            bool b;
            tu_int64 i64;
            double dbl;
            UChar32 chr;
            struct {
                const char *data;
                tu_int32 size;
            } utf8;
            struct {
                tu_uint32 assembly;
                tu_uint32 value;
            } descriptor;
            BaseRef *ref;
        } data;
    };

    bool operator==(const lyric_runtime::DataCell &lhs, const lyric_runtime::DataCell &rhs);

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const DataCell &cell);
}

#endif // LYRIC_RUNTIME_DATA_CELL_H
