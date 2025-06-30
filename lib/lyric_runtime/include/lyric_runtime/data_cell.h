#ifndef LYRIC_RUNTIME_DATA_CELL_H
#define LYRIC_RUNTIME_DATA_CELL_H

#include <unicode/ustring.h>

#include <tempo_utils/log_message.h>

#include "runtime_types.h"
#include "literal_cell.h"

namespace lyric_runtime {

    // forward declarations
    class BaseRef;
    class BytesRef;
    class DescriptorEntry;
    class RestRef;
    class StringRef;
    class TypeEntry;
    class UrlRef;

    enum class DataCellType : uint8_t {
        INVALID,
        NIL,
        UNDEF,
        BOOL,
        I64,
        DBL,
        CHAR32,
        REF,
        BYTES,
        STRING,
        URL,
        REST,
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
        BINDING,
        STATIC,
    };

    struct DataCell {

        DataCell();
        explicit DataCell(bool b);
        explicit DataCell(tu_int64 i64);
        explicit DataCell(double dbl);
        explicit DataCell(char32_t char32);
        DataCell(const DataCell &other);
        DataCell(DataCell &&other) noexcept;
        ~DataCell() = default;

        DataCell& operator=(const DataCell &other);
        DataCell& operator=(DataCell &&other) noexcept;

        bool isValid() const;
        bool isIntrinsic() const;
        bool isDescriptor() const;
        bool isReference() const;

        std::string toString() const;

        static DataCell nil();
        static DataCell undef();

        static DataCell forLiteral(const LiteralCell &literal);
        static DataCell forBytes(BytesRef *bytes);
        static DataCell forDescriptor(DescriptorEntry *descriptor);
        static DataCell forRef(BaseRef *ref);
        static DataCell forRest(RestRef *rest);
        static DataCell forString(StringRef *str);
        static DataCell forType(TypeEntry *type);
        static DataCell forUrl(UrlRef *url);

        DataCellType type;
        union {
            bool b;
            tu_int64 i64;
            double dbl;
            char32_t chr;
            DescriptorEntry *descriptor;
            TypeEntry *type;
            BaseRef *ref;
            BytesRef *bytes;
            StringRef *str;
            RestRef *rest;
            UrlRef *url;
        } data;
    };

    bool operator==(const DataCell &lhs, const DataCell &rhs);

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const DataCell &cell);
}

#endif // LYRIC_RUNTIME_DATA_CELL_H
