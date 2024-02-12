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
        explicit DataCell(int64_t i64);
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
        static DataCell forUtf8(const char *data, int32_t size);
        static DataCell forRef(BaseRef *ref);
        static DataCell forClass(uint32_t assemblyIndex, uint32_t classIndex);
        static DataCell forStruct(uint32_t assemblyIndex, uint32_t structIndex);
        static DataCell forInstance(uint32_t assemblyIndex, uint32_t instanceIndex);
        static DataCell forConcept(uint32_t assemblyIndex, uint32_t conceptIndex);
        static DataCell forEnum(uint32_t assemblyIndex, uint32_t enumIndex);
        static DataCell forCall(uint32_t assemblyIndex, uint32_t callIndex);
        static DataCell forField(uint32_t assemblyIndex, uint32_t fieldIndex);
        static DataCell forAction(uint32_t assemblyIndex, uint32_t actionIndex);
        static DataCell forType(uint32_t assemblyIndex, uint32_t typeIndex);
        static DataCell forExistential(uint32_t assemblyIndex, uint32_t existentialIndex);
        static DataCell forNamespace(uint32_t assemblyIndex, uint32_t namespaceIndex);

        DataCellType type;
        union {
            bool b;
            int64_t i64;
            double dbl;
            UChar32 chr;
            struct {
                const char *data;
                int32_t size;
            } utf8;
            struct {
                uint32_t assembly;
                uint32_t value;
            } descriptor;
            BaseRef *ref;
        } data;
    };

    bool operator==(const lyric_runtime::DataCell &lhs, const lyric_runtime::DataCell &rhs);

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const DataCell &cell);
}

#endif // LYRIC_RUNTIME_DATA_CELL_H
