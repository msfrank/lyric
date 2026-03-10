#include <absl/strings/substitute.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytes_ref.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/descriptor_entry.h>
#include <lyric_runtime/literal_cell.h>
#include <lyric_runtime/protocol_ref.h>
#include <lyric_runtime/rest_ref.h>
#include <lyric_runtime/status_ref.h>
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

lyric_runtime::DataCell::DataCell(char32_t chr)
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
        case DataCellType::BYTES:
            type = other.type;
            data.bytes = other.data.bytes;
            break;
        case DataCellType::STRING:
            type = other.type;
            data.str = other.data.str;
            break;
        case DataCellType::URL:
            type = other.type;
            data.url = other.data.url;
            break;
        case DataCellType::STATUS:
            type = other.type;
            data.status = other.data.status;
            break;
        case DataCellType::REST:
            type = other.type;
            data.rest = other.data.rest;
            break;
        case DataCellType::PROTOCOL:
            type = other.type;
            data.protocol = other.data.protocol;
            break;
        case DataCellType::REF:
            type = other.type;
            data.ref = other.data.ref;
            break;
        case DataCellType::DESCRIPTOR:
            type = other.type;
            data.descriptor = other.data.descriptor;
            break;
        case DataCellType::TYPE:
            type = other.type;
            data.type = other.data.type;
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
        case DataCellType::BYTES:
            data.bytes = other.data.bytes;
            break;
        case DataCellType::STRING:
            data.str = other.data.str;
            break;
        case DataCellType::URL:
            data.url = other.data.url;
            break;
        case DataCellType::STATUS:
            data.status = other.data.status;
            break;
        case DataCellType::PROTOCOL:
            data.protocol = other.data.protocol;
            break;
        case DataCellType::REST:
            data.rest = other.data.rest;
            break;
        case DataCellType::REF:
            data.ref = other.data.ref;
            break;
        case DataCellType::DESCRIPTOR:
            data.descriptor = other.data.descriptor;
            break;
        case DataCellType::TYPE:
            data.type = other.data.type;
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
lyric_runtime::DataCell::forLiteral(const LiteralCell &literal)
{
    DataCell cell;
    switch (literal.type) {
        case LiteralCellType::INVALID:
            cell.type = DataCellType::INVALID;
            break;
        case LiteralCellType::NIL:
            cell.type = DataCellType::NIL;
            break;
        case LiteralCellType::UNDEF:
            cell.type = DataCellType::UNDEF;
            break;
        case LiteralCellType::BOOL:
            cell.type = DataCellType::BOOL;
            cell.data.b = literal.literal.b;
            break;
        case LiteralCellType::I64:
            cell.type = DataCellType::I64;
            cell.data.i64 = literal.literal.i64;
            break;
        case LiteralCellType::DBL:
            cell.type = DataCellType::DBL;
            cell.data.dbl = literal.literal.dbl;
            break;
        case LiteralCellType::CHAR32:
            cell.type = DataCellType::CHAR32;
            cell.data.chr = literal.literal.chr;
            break;
        default:
            TU_UNREACHABLE();
    }
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forDescriptor(DescriptorEntry *descriptor)
{
    TU_ASSERT (descriptor != nullptr);
    DataCell cell;
    cell.type = DataCellType::DESCRIPTOR;
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
lyric_runtime::DataCell::forBytes(BytesRef *bytes)
{
    TU_ASSERT (bytes != nullptr);
    DataCell cell;
    cell.type = DataCellType::BYTES;
    cell.data.bytes = bytes;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forStatus(StatusRef *status)
{
    TU_ASSERT (status != nullptr);
    DataCell cell;
    cell.type = DataCellType::STATUS;
    cell.data.status = status;
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
lyric_runtime::DataCell::forType(TypeEntry *type)
{
    TU_ASSERT (type != nullptr);
    DataCell cell;
    cell.type = DataCellType::TYPE;
    cell.data.type = type;
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

lyric_runtime::DataCell
lyric_runtime::DataCell::forProtocol(ProtocolRef *protocol)
{
    TU_ASSERT (protocol != nullptr);
    DataCell cell;
    cell.type = DataCellType::PROTOCOL;
    cell.data.protocol = protocol;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forRest(RestRef *rest)
{
    TU_ASSERT (rest != nullptr);
    DataCell cell;
    cell.type = DataCellType::REST;
    cell.data.rest = rest;
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
        case DataCellType::BYTES:
            type = other.type;
            data.bytes = other.data.bytes;
            break;
        case DataCellType::STRING:
            type = other.type;
            data.str = other.data.str;
            break;
        case DataCellType::URL:
            type = other.type;
            data.url = other.data.url;
            break;
        case DataCellType::STATUS:
            type = other.type;
            data.status = other.data.status;
            break;
        case DataCellType::PROTOCOL:
            type = other.type;
            data.protocol = other.data.protocol;
            break;
        case DataCellType::REST:
            type = other.type;
            data.rest = other.data.rest;
            break;
        case DataCellType::REF:
            type = other.type;
            data.ref = other.data.ref;
            break;
        case DataCellType::DESCRIPTOR:
            type = other.type;
            data.descriptor = other.data.descriptor;
            break;
        case DataCellType::TYPE:
            type = other.type;
            data.type = other.data.type;
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
            case DataCellType::BYTES:
                data.bytes = other.data.bytes;
                break;
            case DataCellType::STRING:
                data.str = other.data.str;
                break;
            case DataCellType::URL:
                data.url = other.data.url;
                break;
            case DataCellType::STATUS:
                data.status = other.data.status;
                break;
            case DataCellType::PROTOCOL:
                data.protocol = other.data.protocol;
                break;
            case DataCellType::REST:
                data.rest = other.data.rest;
                break;
            case DataCellType::REF:
                data.ref = other.data.ref;
                break;
            case DataCellType::DESCRIPTOR:
                data.descriptor = other.data.descriptor;
                break;
            case DataCellType::TYPE:
                data.type = other.data.type;
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
        case DataCellType::BOOL:
        case DataCellType::BYTES:
        case DataCellType::CHAR32:
        case DataCellType::DESCRIPTOR:
        case DataCellType::DBL:
        case DataCellType::I64:
        case DataCellType::NIL:
        case DataCellType::PROTOCOL:
        case DataCellType::REF:
        case DataCellType::REST:
        case DataCellType::STATUS:
        case DataCellType::STRING:
        case DataCellType::TYPE:
        case DataCellType::UNDEF:
        case DataCellType::URL:
            return true;
        default:
            return false;
    }
}

bool
lyric_runtime::DataCell::isIntrinsic() const
{
    switch (type) {
        case DataCellType::BOOL:
        case DataCellType::CHAR32:
        case DataCellType::DBL:
        case DataCellType::I64:
        case DataCellType::NIL:
        case DataCellType::UNDEF:
            return true;
        default:
            return false;
    }
}

bool
lyric_runtime::DataCell::isDescriptor() const
{
    switch (type) {
        case DataCellType::DESCRIPTOR:
        case DataCellType::TYPE:
            return true;
        default:
            break;
    }
    return false;
}

bool
lyric_runtime::DataCell::isReference() const
{
    switch (type) {
        case DataCellType::BYTES:
        case DataCellType::PROTOCOL:
        case DataCellType::REF:
        case DataCellType::REST:
        case DataCellType::STATUS:
        case DataCellType::STRING:
        case DataCellType::URL:
            return true;
        default:
            break;
    }
    return false;
}

lyric_object::LinkageSection
lyric_runtime::DataCell::getLinkage() const
{
    if (type != DataCellType::DESCRIPTOR)
        return lyric_object::LinkageSection::Invalid;
    return data.descriptor->getLinkageSection();
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
        case DataCellType::BYTES:
            return data.bytes->toString();
        case DataCellType::STRING:
            return data.str->toString();
        case DataCellType::URL:
            return data.url->toString();
        case DataCellType::STATUS:
            return data.status->toString();
        case DataCellType::PROTOCOL:
            return data.protocol->toString();
        case DataCellType::REST:
            return data.rest->toString();
        case DataCellType::REF:
            return data.ref->toString();
        case DataCellType::DESCRIPTOR:
            return absl::Substitute("<object=$0, descriptor=$1, offset=$2>",
                data.descriptor->getSegmentIndex(),
                lyric_object::linkage_section_to_name(data.descriptor->getLinkageSection()),
                data.descriptor->getDescriptorIndex());
        case DataCellType::TYPE:
            return absl::Substitute("<object=$0, type=$1, offset=$2>",
                data.type->getSegmentIndex(),
                data.type->getTypeDef().toString(),
                data.type->getDescriptorIndex());
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
        case DataCellType::BYTES:
            return lhs.data.bytes == rhs.data.bytes;
        case DataCellType::STRING:
            return lhs.data.str == rhs.data.str;
        case DataCellType::URL:
            return lhs.data.url == rhs.data.url;
        case DataCellType::STATUS:
            return lhs.data.status == rhs.data.status;
        case DataCellType::PROTOCOL:
            return lhs.data.protocol == rhs.data.protocol;
        case DataCellType::REST:
            return lhs.data.rest == rhs.data.rest;
        case DataCellType::REF:
            return lhs.data.ref == rhs.data.ref;
        case DataCellType::DESCRIPTOR:
            return lhs.data.descriptor->getSegmentIndex() == rhs.data.descriptor->getSegmentIndex()
                && lhs.data.descriptor->getLinkageSection() == rhs.data.descriptor->getLinkageSection()
                && lhs.data.descriptor->getDescriptorIndex() == rhs.data.descriptor->getDescriptorIndex();
        case DataCellType::TYPE:
            return lhs.data.type->getSegmentIndex() == rhs.data.type->getSegmentIndex()
                && lhs.data.type->getDescriptorIndex() == rhs.data.type->getDescriptorIndex();
    }

    TU_UNREACHABLE();
}
bool
lyric_runtime::operator!=(const DataCell &lhs, const DataCell &rhs)
{
    return !(lhs == rhs);
}

tempo_utils::LogMessage&&
lyric_runtime::operator<<(tempo_utils::LogMessage &&message, const DataCell &cell)
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
        case DataCellType::BYTES:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(bytes=$0)", cell.data.bytes->toString());
            break;
        case DataCellType::STRING:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(string=$0)", cell.data.str->toString());
            break;
        case DataCellType::URL:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(url=$0)", cell.data.url->toString());
            break;
        case DataCellType::STATUS:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(status=$0)", cell.data.status->toString());
            break;
        case DataCellType::PROTOCOL:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(protocol=$0)", cell.data.protocol->toString());
            break;
        case DataCellType::REST:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(rest=$0)", cell.data.rest->toString());
            break;
        case DataCellType::REF:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(ref=$0)", cell.data.ref->toString());
            break;
        case DataCellType::DESCRIPTOR:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, descriptor=$1, offset=$2)",
                    cell.data.descriptor->getSegmentIndex(),
                    lyric_object::linkage_section_to_name(cell.data.descriptor->getLinkageSection()),
                    cell.data.descriptor->getDescriptorIndex());
            break;
        case DataCellType::TYPE:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, type=$1, offset=$2)",
                    cell.data.type->getSegmentIndex(),
                    cell.data.type->getTypeDef().toString(),
                    cell.data.type->getDescriptorIndex());
            break;
        case DataCellType::INVALID:
        default:
            std::forward<tempo_utils::LogMessage>(message) << "DataCell(INVALID)";
            break;
    }
    return std::move(message);
}
