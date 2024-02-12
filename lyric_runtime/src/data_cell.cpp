#include <absl/strings/substitute.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/literal_cell.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

lyric_runtime::DataCell::DataCell()
{
    type = DataCellType::INVALID;
}

lyric_runtime::DataCell::DataCell(bool b)
{
    type = DataCellType::BOOL;
    data.b = b;
}

lyric_runtime::DataCell::DataCell(int64_t i64)
{
    type = DataCellType::I64;
    data.i64 = i64;
}

lyric_runtime::DataCell::DataCell(double dbl)
{
    type = DataCellType::DBL;
    data.dbl = dbl;
}

lyric_runtime::DataCell::DataCell(UChar32 chr)
{
    type = DataCellType::CHAR32;
    data.chr = chr;
}

lyric_runtime::DataCell::DataCell(const DataCell &other) : DataCell()
{
    switch (other.type) {
        case DataCellType::INVALID:
        case DataCellType::NIL:
        case DataCellType::PRESENT:
            type = other.type;
            break;
        case DataCellType::BOOL:
            type = other.type;
            data.b = other.data.b;
            break;
        case DataCellType::I64:
            type = other.type;
            data.i64 = other.data.i64;
            break;
        case DataCellType::DBL:
            type = other.type;
            data.dbl = other.data.dbl;
            break;
        case DataCellType::CHAR32:
            type = other.type;
            data.chr = other.data.chr;
            break;
        case DataCellType::UTF8:
            type = other.type;
            data.utf8.data = other.data.utf8.data;
            data.utf8.size = other.data.utf8.size;
            break;
        case DataCellType::REF:
            type = other.type;
            data.ref = other.data.ref;
            break;
        case DataCellType::TYPE:
        case DataCellType::CLASS:
        case DataCellType::STRUCT:
        case DataCellType::INSTANCE:
        case DataCellType::CONCEPT:
        case DataCellType::ENUM:
        case DataCellType::FIELD:
        case DataCellType::CALL:
        case DataCellType::ACTION:
        case DataCellType::EXISTENTIAL:
        case DataCellType::NAMESPACE:
            type = other.type;
            data.descriptor = other.data.descriptor;
            break;
        default:
            type = DataCellType::INVALID;
            break;
    }
}

lyric_runtime::DataCell::DataCell(DataCell &&other) noexcept : DataCell()
{
    type = other.type;
    switch (other.type) {
        case DataCellType::BOOL:
            data.b = other.data.b;
            break;
        case DataCellType::I64:
            data.i64 = other.data.i64;
            break;
        case DataCellType::DBL:
            data.dbl = other.data.dbl;
            break;
        case DataCellType::CHAR32:
            data.chr = other.data.chr;
            break;
        case DataCellType::UTF8:
            data.utf8.data = other.data.utf8.data;
            data.utf8.size = other.data.utf8.size;
            break;
        case DataCellType::REF:
            data.ref = other.data.ref;
            break;
        case DataCellType::TYPE:
        case DataCellType::CLASS:
        case DataCellType::STRUCT:
        case DataCellType::INSTANCE:
        case DataCellType::CONCEPT:
        case DataCellType::ENUM:
        case DataCellType::FIELD:
        case DataCellType::CALL:
        case DataCellType::ACTION:
        case DataCellType::EXISTENTIAL:
        case DataCellType::NAMESPACE:
            data.descriptor = other.data.descriptor;
            break;
        default:
            break;
    }
    other.type = DataCellType::INVALID;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::nil()
{
    DataCell cell;
    cell.type = DataCellType::NIL;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::present()
{
    DataCell cell;
    cell.type = DataCellType::PRESENT;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forLiteral(const lyric_runtime::LiteralCell &literal)
{
    DataCell cell;
    switch (literal.type) {
        case lyric_runtime::LiteralCellType::INVALID:
            cell.type = DataCellType::INVALID;
            break;
        case lyric_runtime::LiteralCellType::NIL:
            cell.type = DataCellType::NIL;
            break;
        case lyric_runtime::LiteralCellType::PRESENT:
            cell.type = DataCellType::PRESENT;
            break;
        case lyric_runtime::LiteralCellType::BOOL:
            cell.type = DataCellType::BOOL;
            cell.data.b = literal.literal.b;
            break;
        case lyric_runtime::LiteralCellType::I64:
            cell.type = DataCellType::I64;
            cell.data.i64 = literal.literal.i64;
            break;
        case lyric_runtime::LiteralCellType::DBL:
            cell.type = DataCellType::DBL;
            cell.data.dbl = literal.literal.dbl;
            break;
        case lyric_runtime::LiteralCellType::CHAR32:
            cell.type = DataCellType::CHAR32;
            cell.data.chr = literal.literal.chr;
            break;
        case lyric_runtime::LiteralCellType::UTF8:
            cell.type = DataCellType::UTF8;
            cell.data.utf8.data = literal.literal.utf8.data;
            cell.data.utf8.size = literal.literal.utf8.size;
            break;
        default:
            TU_UNREACHABLE();
    }
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forUtf8(const char *data, int32_t size)
{
    TU_ASSERT (data != nullptr);
    TU_ASSERT (size >= 0);
    DataCell cell;
    cell.type = DataCellType::UTF8;
    cell.data.utf8.data = data;
    cell.data.utf8.size = size;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forRef(BaseRef *instance)
{
    TU_ASSERT (instance != nullptr);
    DataCell cell;
    cell.type = DataCellType::REF;
    cell.data.ref = instance;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forClass(uint32_t assemblyIndex, uint32_t classIndex)
{
    TU_ASSERT (assemblyIndex != INVALID_ADDRESS_U32);
    DataCell cell;
    cell.type = DataCellType::CLASS;
    cell.data.descriptor.assembly = assemblyIndex;
    cell.data.descriptor.value = classIndex;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forStruct(uint32_t assemblyIndex, uint32_t structIndex)
{
    TU_ASSERT (assemblyIndex != INVALID_ADDRESS_U32);
    DataCell cell;
    cell.type = DataCellType::STRUCT;
    cell.data.descriptor.assembly = assemblyIndex;
    cell.data.descriptor.value = structIndex;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forEnum(uint32_t assemblyIndex, uint32_t enumIndex)
{
    TU_ASSERT (assemblyIndex != INVALID_ADDRESS_U32);
    DataCell cell;
    cell.type = DataCellType::ENUM;
    cell.data.descriptor.assembly = assemblyIndex;
    cell.data.descriptor.value = enumIndex;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forInstance(uint32_t assemblyIndex, uint32_t instanceIndex)
{
    TU_ASSERT (assemblyIndex != INVALID_ADDRESS_U32);
    DataCell cell;
    cell.type = DataCellType::INSTANCE;
    cell.data.descriptor.assembly = assemblyIndex;
    cell.data.descriptor.value = instanceIndex;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forConcept(uint32_t assemblyIndex, uint32_t conceptIndex)
{
    TU_ASSERT (assemblyIndex != INVALID_ADDRESS_U32);
    DataCell cell;
    cell.type = DataCellType::CONCEPT;
    cell.data.descriptor.assembly = assemblyIndex;
    cell.data.descriptor.value = conceptIndex;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forField(uint32_t assemblyIndex, uint32_t fieldIndex)
{
    TU_ASSERT (assemblyIndex != INVALID_ADDRESS_U32);
    DataCell cell;
    cell.type = DataCellType::FIELD;
    cell.data.descriptor.assembly = assemblyIndex;
    cell.data.descriptor.value = fieldIndex;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forCall(uint32_t assemblyIndex, uint32_t callIndex)
{
    TU_ASSERT (assemblyIndex != INVALID_ADDRESS_U32);
    DataCell cell;
    cell.type = DataCellType::CALL;
    cell.data.descriptor.assembly = assemblyIndex;
    cell.data.descriptor.value = callIndex;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forAction(uint32_t assemblyIndex, uint32_t actionIndex)
{
    TU_ASSERT (assemblyIndex != INVALID_ADDRESS_U32);
    DataCell cell;
    cell.type = DataCellType::ACTION;
    cell.data.descriptor.assembly = assemblyIndex;
    cell.data.descriptor.value = actionIndex;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forType(uint32_t assemblyIndex, uint32_t typeIndex)
{
    TU_ASSERT (assemblyIndex != INVALID_ADDRESS_U32);
    DataCell cell;
    cell.type = DataCellType::TYPE;
    cell.data.descriptor.assembly = assemblyIndex;
    cell.data.descriptor.value = typeIndex;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forExistential(uint32_t assemblyIndex, uint32_t existentialIndex)
{
    TU_ASSERT (assemblyIndex != INVALID_ADDRESS_U32);
    DataCell cell;
    cell.type = DataCellType::EXISTENTIAL;
    cell.data.descriptor.assembly = assemblyIndex;
    cell.data.descriptor.value = existentialIndex;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forNamespace(uint32_t assemblyIndex, uint32_t namespaceIndex)
{
    TU_ASSERT (assemblyIndex != INVALID_ADDRESS_U32);
    DataCell cell;
    cell.type = DataCellType::NAMESPACE;
    cell.data.descriptor.assembly = assemblyIndex;
    cell.data.descriptor.value = namespaceIndex;
    return cell;
}

lyric_runtime::DataCell&
lyric_runtime::DataCell::operator=(const DataCell &other)
{
    switch (other.type) {
        case DataCellType::INVALID:
        case DataCellType::NIL:
        case DataCellType::PRESENT:
            type = other.type;
            break;
        case DataCellType::BOOL:
            type = other.type;
            data.b = other.data.b;
            break;
        case DataCellType::I64:
            type = other.type;
            data.i64 = other.data.i64;
            break;
        case DataCellType::DBL:
            type = other.type;
            data.dbl = other.data.dbl;
            break;
        case DataCellType::CHAR32:
            type = other.type;
            data.chr = other.data.chr;
            break;
        case DataCellType::UTF8:
            type = other.type;
            data.utf8.data = other.data.utf8.data;
            data.utf8.size = other.data.utf8.size;
            break;
        case DataCellType::REF:
            type = other.type;
            data.ref = other.data.ref;
            break;
        case DataCellType::TYPE:
        case DataCellType::CLASS:
        case DataCellType::STRUCT:
        case DataCellType::INSTANCE:
        case DataCellType::CONCEPT:
        case DataCellType::ENUM:
        case DataCellType::FIELD:
        case DataCellType::CALL:
        case DataCellType::ACTION:
        case DataCellType::EXISTENTIAL:
        case DataCellType::NAMESPACE:
            type = other.type;
            data.descriptor = other.data.descriptor;
            break;
        default:
            type = DataCellType::INVALID;
            break;
    }
    return *this;
}

lyric_runtime::DataCell&
lyric_runtime::DataCell::operator=(DataCell &&other) noexcept
{
    if (this != &other) {
        type = other.type;
        switch (other.type) {
            case DataCellType::BOOL:
                data.b = other.data.b;
                break;
            case DataCellType::I64:
                data.i64 = other.data.i64;
                break;
            case DataCellType::DBL:
                data.dbl = other.data.dbl;
                break;
            case DataCellType::CHAR32:
                data.chr = other.data.chr;
                break;
            case DataCellType::UTF8:
                data.utf8.data = other.data.utf8.data;
                data.utf8.size = other.data.utf8.size;
                break;
            case DataCellType::REF:
                data.ref = other.data.ref;
                break;
            case DataCellType::TYPE:
            case DataCellType::CLASS:
            case DataCellType::STRUCT:
            case DataCellType::INSTANCE:
            case DataCellType::CONCEPT:
            case DataCellType::ENUM:
            case DataCellType::FIELD:
            case DataCellType::CALL:
            case DataCellType::ACTION:
            case DataCellType::EXISTENTIAL:
            case DataCellType::NAMESPACE:
                data.descriptor = other.data.descriptor;
                break;
            default:
                break;
        }
        other.type = DataCellType::INVALID;
    }

    return *this;
}

bool
lyric_runtime::DataCell::isValid() const
{
    switch (type) {
        case DataCellType::NIL:
        case DataCellType::PRESENT:
        case DataCellType::BOOL:
        case DataCellType::I64:
        case DataCellType::DBL:
        case DataCellType::CHAR32:
        case DataCellType::UTF8:
        case DataCellType::REF:
        case DataCellType::TYPE:
        case DataCellType::CLASS:
        case DataCellType::STRUCT:
        case DataCellType::INSTANCE:
        case DataCellType::CONCEPT:
        case DataCellType::ENUM:
        case DataCellType::FIELD:
        case DataCellType::CALL:
        case DataCellType::ACTION:
        case DataCellType::EXISTENTIAL:
        case DataCellType::NAMESPACE:
            return true;
        default:
            break;
    }
    return false;
}

std::string
lyric_runtime::DataCell::toString() const
{
    switch (type) {
        case DataCellType::NIL:
            return "Nil";
        case DataCellType::PRESENT:
            return "Present";
        case DataCellType::BOOL:
            return data.b ? "true" : "false";
        case DataCellType::I64:
            return absl::StrCat(data.i64);
        case DataCellType::DBL:
            return absl::StrCat(data.dbl);
        case DataCellType::CHAR32:
            return tempo_utils::convert_to_utf8(data.chr);
        case DataCellType::UTF8:
            return std::string(data.utf8.data, data.utf8.size);
        case DataCellType::REF:
            return data.ref->toString();
        case DataCellType::CLASS:
            return absl::Substitute("<assembly=$0, class=$1>",
                data.descriptor.assembly, data.descriptor.value);
        case DataCellType::STRUCT:
            return absl::Substitute("<assembly=$0, struct=$1>",
                data.descriptor.assembly, data.descriptor.value);
        case DataCellType::INSTANCE:
            return absl::Substitute("<assembly=$0, instance=$1>",
                data.descriptor.assembly, data.descriptor.value);
        case DataCellType::CONCEPT:
            return absl::Substitute("<assembly=$0, concept=$1>",
                data.descriptor.assembly, data.descriptor.value);
        case DataCellType::ENUM:
            return absl::Substitute("<assembly=$0, enum=$1>",
                data.descriptor.assembly, data.descriptor.value);
        case DataCellType::FIELD:
            return absl::Substitute("<assembly=$0, field=$1>",
                data.descriptor.assembly, data.descriptor.value);
        case DataCellType::CALL:
            return absl::Substitute("<assembly=$0, call=$1>",
                data.descriptor.assembly, data.descriptor.value);
        case DataCellType::ACTION:
            return absl::Substitute("<assembly=$0, action=$1>",
                data.descriptor.assembly, data.descriptor.value);
        case DataCellType::TYPE:
            return absl::Substitute("<assembly=$0, type=$1>",
                data.descriptor.assembly, data.descriptor.value);
        case DataCellType::EXISTENTIAL:
            return absl::Substitute("<assembly=$0, existential=$1>",
                data.descriptor.assembly, data.descriptor.value);
        case DataCellType::NAMESPACE:
            return absl::Substitute("<assembly=$0, namespace=$1>",
                data.descriptor.assembly, data.descriptor.value);
        case DataCellType::INVALID:
        default:
            break;
    }
    return "<INVALID>";
}

bool
lyric_runtime::operator==(const DataCell &lhs, const DataCell &rhs)
{
    if (lhs.type != rhs.type)
        return false;

    switch (lhs.type) {
        case DataCellType::INVALID:
        case DataCellType::NIL:
        case DataCellType::PRESENT:
            return true;
        case DataCellType::BOOL:
            return lhs.data.b == rhs.data.b;
        case DataCellType::I64:
            return lhs.data.i64 == rhs.data.i64;
        case DataCellType::DBL:
            return lhs.data.dbl == rhs.data.dbl;
        case DataCellType::CHAR32:
            return lhs.data.chr == rhs.data.chr;
        case DataCellType::UTF8:
            if (lhs.data.utf8.size != rhs.data.utf8.size)
                return false;
            return std::memcmp(lhs.data.utf8.data, rhs.data.utf8.data, lhs.data.utf8.size) == 0;
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
            return lhs.data.descriptor.assembly == rhs.data.descriptor.assembly
                && lhs.data.descriptor.value == rhs.data.descriptor.value;
        case DataCellType::REF:
            return lhs.data.ref == rhs.data.ref;
    }

    TU_UNREACHABLE();
}

tempo_utils::LogMessage&&
lyric_runtime::operator<<(tempo_utils::LogMessage &&message, const lyric_runtime::DataCell &cell)
{
    switch (cell.type) {
        case DataCellType::NIL:
            std::forward<tempo_utils::LogMessage>(message) << "DataCell(Nil)";
            break;
        case DataCellType::PRESENT:
            std::forward<tempo_utils::LogMessage>(message) << "DataCell(Present)";
            break;
        case DataCellType::BOOL:
            std::forward<tempo_utils::LogMessage>(message)
                << (cell.data.b ? "DataCell(boolean=true)" : "DataCell(boolean=false)");
            break;
        case DataCellType::I64:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::StrCat("DataCell(integer=", cell.data.i64, ")");
            break;
        case DataCellType::DBL:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::StrCat("DataCell(float=", cell.data.dbl, ")");
            break;
        case DataCellType::CHAR32:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(char=$0)",
                    tempo_utils::convert_to_utf8(cell.data.chr));
            break;
        case DataCellType::UTF8:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(utf8=$0)",
                    std::string(cell.data.utf8.data, cell.data.utf8.size));
            break;
        case DataCellType::REF:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(ref=$0)", cell.data.ref->toString());
            break;
        case DataCellType::CLASS:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(assembly=$0, class=$1)",
                    cell.data.descriptor.assembly, cell.data.descriptor.value);
            break;
        case DataCellType::STRUCT:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(assembly=$0, struct=$1)",
                    cell.data.descriptor.assembly, cell.data.descriptor.value);
            break;
        case DataCellType::INSTANCE:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(assembly=$0, instance=$1)",
                    cell.data.descriptor.assembly, cell.data.descriptor.value);
            break;
        case DataCellType::CONCEPT:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(assembly=$0, concept=$1)",
                    cell.data.descriptor.assembly, cell.data.descriptor.value);
            break;
        case DataCellType::ENUM:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(assembly=$0, enum=$1)",
                    cell.data.descriptor.assembly, cell.data.descriptor.value);
            break;
        case DataCellType::FIELD:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(assembly=$0, field=$1)",
                    cell.data.descriptor.assembly, cell.data.descriptor.value);
            break;
        case DataCellType::CALL:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(assembly=$0, call=$1)",
                    cell.data.descriptor.assembly, cell.data.descriptor.value);
            break;
        case DataCellType::ACTION:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(assembly=$0, action=$1)",
                    cell.data.descriptor.assembly, cell.data.descriptor.value);
            break;
        case DataCellType::TYPE:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(assembly=$0, type=$1)",
                    cell.data.descriptor.assembly, cell.data.descriptor.value);
            break;
        case DataCellType::EXISTENTIAL:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(assembly=$0, existential=$1)",
                    cell.data.descriptor.assembly, cell.data.descriptor.value);
            break;
        case DataCellType::NAMESPACE:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(assembly=$0, namespace=$1)",
                    cell.data.descriptor.assembly, cell.data.descriptor.value);
            break;
        case DataCellType::INVALID:
        default:
            std::forward<tempo_utils::LogMessage>(message) << "DataCell(INVALID)";
            break;
    }
    return std::move(message);
}
