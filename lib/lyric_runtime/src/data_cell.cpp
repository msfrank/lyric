#include <absl/strings/substitute.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/descriptor_entry.h>
#include <lyric_runtime/literal_cell.h>
#include <lyric_runtime/string_ref.h>
#include <lyric_runtime/url_ref.h>
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

lyric_runtime::DataCell::DataCell(tu_int64 i64)
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
        case DataCellType::UNDEF:
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
        case DataCellType::STRING:
            type = other.type;
            data.str = other.data.str;
            break;
        case DataCellType::URL:
            type = other.type;
            data.url = other.data.url;
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
        case DataCellType::STRING:
            data.str = other.data.str;
            break;
        case DataCellType::URL:
            data.url = other.data.url;
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
lyric_runtime::DataCell::undef()
{
    DataCell cell;
    cell.type = DataCellType::UNDEF;
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
        case lyric_runtime::LiteralCellType::UNDEF:
            cell.type = DataCellType::UNDEF;
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
        default:
            TU_UNREACHABLE();
    }
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forDescriptor(lyric_runtime::DescriptorEntry *descriptor)
{
    TU_ASSERT (descriptor != nullptr);
    DataCell cell;
    switch (descriptor->getLinkageSection()) {
        case lyric_object::LinkageSection::Action:
            cell.type = DataCellType::ACTION;
            break;
        case lyric_object::LinkageSection::Call:
            cell.type = DataCellType::CALL;
            break;
        case lyric_object::LinkageSection::Class:
            cell.type = DataCellType::CLASS;
            break;
        case lyric_object::LinkageSection::Concept:
            cell.type = DataCellType::CONCEPT;
            break;
        case lyric_object::LinkageSection::Enum:
            cell.type = DataCellType::ENUM;
            break;
        case lyric_object::LinkageSection::Existential:
            cell.type = DataCellType::EXISTENTIAL;
            break;
        case lyric_object::LinkageSection::Field:
            cell.type = DataCellType::FIELD;
            break;
        case lyric_object::LinkageSection::Instance:
            cell.type = DataCellType::INSTANCE;
            break;
        case lyric_object::LinkageSection::Namespace:
            cell.type = DataCellType::NAMESPACE;
            break;
        case lyric_object::LinkageSection::Struct:
            cell.type = DataCellType::STRUCT;
            break;
        case lyric_object::LinkageSection::Type:
            cell.type = DataCellType::TYPE;
            break;
        default:
            return {};
    }
    cell.data.descriptor = descriptor;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forRef(BaseRef *ref)
{
    TU_ASSERT (ref != nullptr);
    DataCell cell;
    cell.type = DataCellType::REF;
    cell.data.ref = ref;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forString(StringRef *str)
{
    TU_ASSERT (str != nullptr);
    DataCell cell;
    cell.type = DataCellType::STRING;
    cell.data.str = str;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forUrl(UrlRef *url)
{
    TU_ASSERT (url != nullptr);
    DataCell cell;
    cell.type = DataCellType::URL;
    cell.data.url = url;
    return cell;
}

lyric_runtime::DataCell&
lyric_runtime::DataCell::operator=(const DataCell &other)
{
    switch (other.type) {
        case DataCellType::INVALID:
        case DataCellType::NIL:
        case DataCellType::UNDEF:
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
        case DataCellType::STRING:
            type = other.type;
            data.str = other.data.str;
            break;
        case DataCellType::URL:
            type = other.type;
            data.url = other.data.url;
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
            case DataCellType::STRING:
                data.str = other.data.str;
                break;
            case DataCellType::URL:
                data.url = other.data.url;
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
        case DataCellType::UNDEF:
        case DataCellType::BOOL:
        case DataCellType::I64:
        case DataCellType::DBL:
        case DataCellType::CHAR32:
        case DataCellType::STRING:
        case DataCellType::URL:
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
            return "nil";
        case DataCellType::UNDEF:
            return "undef";
        case DataCellType::BOOL:
            return data.b ? "true" : "false";
        case DataCellType::I64:
            return absl::StrCat(data.i64);
        case DataCellType::DBL:
            return absl::StrCat(data.dbl);
        case DataCellType::CHAR32:
            return tempo_utils::convert_to_utf8(data.chr);
        case DataCellType::STRING:
            return data.str->toString();
        case DataCellType::URL:
            return data.url->toString();
        case DataCellType::REF:
            return data.ref->toString();
        case DataCellType::CLASS:
            return absl::Substitute("<object=$0, class=$1>",
                data.descriptor->getSegmentIndex(), data.descriptor->getDescriptorIndex());
        case DataCellType::STRUCT:
            return absl::Substitute("<object=$0, struct=$1>",
                data.descriptor->getSegmentIndex(), data.descriptor->getDescriptorIndex());
        case DataCellType::INSTANCE:
            return absl::Substitute("<object=$0, instance=$1>",
                data.descriptor->getSegmentIndex(), data.descriptor->getDescriptorIndex());
        case DataCellType::CONCEPT:
            return absl::Substitute("<object=$0, concept=$1>",
                data.descriptor->getSegmentIndex(), data.descriptor->getDescriptorIndex());
        case DataCellType::ENUM:
            return absl::Substitute("<object=$0, enum=$1>",
                data.descriptor->getSegmentIndex(), data.descriptor->getDescriptorIndex());
        case DataCellType::FIELD:
            return absl::Substitute("<object=$0, field=$1>",
                data.descriptor->getSegmentIndex(), data.descriptor->getDescriptorIndex());
        case DataCellType::CALL:
            return absl::Substitute("<object=$0, call=$1>",
                data.descriptor->getSegmentIndex(), data.descriptor->getDescriptorIndex());
        case DataCellType::ACTION:
            return absl::Substitute("<object=$0, action=$1>",
                data.descriptor->getSegmentIndex(), data.descriptor->getDescriptorIndex());
        case DataCellType::TYPE:
            return absl::Substitute("<object=$0, type=$1>",
                data.descriptor->getSegmentIndex(), data.descriptor->getDescriptorIndex());
        case DataCellType::EXISTENTIAL:
            return absl::Substitute("<object=$0, existential=$1>",
                data.descriptor->getSegmentIndex(), data.descriptor->getDescriptorIndex());
        case DataCellType::NAMESPACE:
            return absl::Substitute("<object=$0, namespace=$1>",
                data.descriptor->getSegmentIndex(), data.descriptor->getDescriptorIndex());
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
        case DataCellType::UNDEF:
            return true;
        case DataCellType::BOOL:
            return lhs.data.b == rhs.data.b;
        case DataCellType::I64:
            return lhs.data.i64 == rhs.data.i64;
        case DataCellType::DBL:
            return lhs.data.dbl == rhs.data.dbl;
        case DataCellType::CHAR32:
            return lhs.data.chr == rhs.data.chr;
        case DataCellType::STRING:
            return lhs.data.str == rhs.data.str;
        case DataCellType::URL:
            return lhs.data.url == rhs.data.url;
        case DataCellType::REF:
            return lhs.data.ref == rhs.data.ref;
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
            return lhs.data.descriptor->getSegmentIndex() == rhs.data.descriptor->getSegmentIndex()
                && lhs.data.descriptor->getDescriptorIndex() == rhs.data.descriptor->getDescriptorIndex();
    }

    TU_UNREACHABLE();
}

tempo_utils::LogMessage&&
lyric_runtime::operator<<(tempo_utils::LogMessage &&message, const lyric_runtime::DataCell &cell)
{
    switch (cell.type) {
        case DataCellType::NIL:
            std::forward<tempo_utils::LogMessage>(message) << "DataCell(nil)";
            break;
        case DataCellType::UNDEF:
            std::forward<tempo_utils::LogMessage>(message) << "DataCell(undef)";
            break;
        case DataCellType::BOOL:
            std::forward<tempo_utils::LogMessage>(message)
                << (cell.data.b ? "DataCell(boolean=true)" : "DataCell(bool=false)");
            break;
        case DataCellType::I64:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::StrCat("DataCell(int=", cell.data.i64, ")");
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
        case DataCellType::STRING:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(string=$0)", cell.data.str->toString());
            break;
        case DataCellType::URL:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(url=$0)", cell.data.url->toString());
            break;
        case DataCellType::REF:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(ref=$0)", cell.data.ref->toString());
            break;
        case DataCellType::CLASS:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, class=$1)",
                    cell.data.descriptor->getSegmentIndex(), cell.data.descriptor->getDescriptorIndex());
            break;
        case DataCellType::STRUCT:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, struct=$1)",
                    cell.data.descriptor->getSegmentIndex(), cell.data.descriptor->getDescriptorIndex());
            break;
        case DataCellType::INSTANCE:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, instance=$1)",
                    cell.data.descriptor->getSegmentIndex(), cell.data.descriptor->getDescriptorIndex());
            break;
        case DataCellType::CONCEPT:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, concept=$1)",
                    cell.data.descriptor->getSegmentIndex(), cell.data.descriptor->getDescriptorIndex());
            break;
        case DataCellType::ENUM:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, enum=$1)",
                    cell.data.descriptor->getSegmentIndex(), cell.data.descriptor->getDescriptorIndex());
            break;
        case DataCellType::FIELD:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, field=$1)",
                    cell.data.descriptor->getSegmentIndex(), cell.data.descriptor->getDescriptorIndex());
            break;
        case DataCellType::CALL:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, call=$1)",
                    cell.data.descriptor->getSegmentIndex(), cell.data.descriptor->getDescriptorIndex());
            break;
        case DataCellType::ACTION:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, action=$1)",
                    cell.data.descriptor->getSegmentIndex(), cell.data.descriptor->getDescriptorIndex());
            break;
        case DataCellType::TYPE:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, type=$1)",
                    cell.data.descriptor->getSegmentIndex(), cell.data.descriptor->getDescriptorIndex());
            break;
        case DataCellType::EXISTENTIAL:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, existential=$1)",
                    cell.data.descriptor->getSegmentIndex(), cell.data.descriptor->getDescriptorIndex());
            break;
        case DataCellType::NAMESPACE:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, namespace=$1)",
                    cell.data.descriptor->getSegmentIndex(), cell.data.descriptor->getDescriptorIndex());
            break;
        case DataCellType::INVALID:
        default:
            std::forward<tempo_utils::LogMessage>(message) << "DataCell(INVALID)";
            break;
    }
    return std::move(message);
}
