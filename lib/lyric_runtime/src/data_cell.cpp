#include <absl/strings/substitute.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytes_ref.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/descriptor_entry.h>
#include <lyric_runtime/namespace_ref.h>
#include <lyric_runtime/protocol_ref.h>
#include <lyric_runtime/rest_ref.h>
#include <lyric_runtime/status_ref.h>
#include <lyric_runtime/string_ref.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

lyric_runtime::DataCell::DataCell()
{
    type = DataCellType::Invalid;
}

lyric_runtime::DataCell::DataCell(bool b)
{
    type = DataCellType::Bool;
    data.b = b;
}

lyric_runtime::DataCell::DataCell(tu_int64 i64)
{
    type = DataCellType::Int64;
    data.i64 = i64;
}

lyric_runtime::DataCell::DataCell(double dbl)
{
    type = DataCellType::Float64;
    data.dbl = dbl;
}

lyric_runtime::DataCell::DataCell(char32_t chr)
{
    type = DataCellType::Char32;
    data.chr = chr;
}

lyric_runtime::DataCell::DataCell(const DataCell &other) : DataCell()
{
    switch (other.type) {
        case DataCellType::Invalid:
        case DataCellType::Nil:
        case DataCellType::Undef:
            type = other.type;
            break;
        case DataCellType::Bool:
            type = other.type;
            data.b = other.data.b;
            break;
        case DataCellType::Int64:
            type = other.type;
            data.i64 = other.data.i64;
            break;
        case DataCellType::Float64:
            type = other.type;
            data.dbl = other.data.dbl;
            break;
        case DataCellType::Char32:
            type = other.type;
            data.chr = other.data.chr;
            break;
        case DataCellType::Bytes:
            type = other.type;
            data.bytes = other.data.bytes;
            break;
        case DataCellType::String:
            type = other.type;
            data.str = other.data.str;
            break;
        case DataCellType::Status:
            type = other.type;
            data.status = other.data.status;
            break;
        case DataCellType::Rest:
            type = other.type;
            data.rest = other.data.rest;
            break;
        case DataCellType::Namespace:
            type = other.type;
            data.ns = other.data.ns;
            break;
        case DataCellType::Protocol:
            type = other.type;
            data.protocol = other.data.protocol;
            break;
        case DataCellType::Ref:
            type = other.type;
            data.ref = other.data.ref;
            break;
        case DataCellType::Descriptor:
            type = other.type;
            data.descriptor = other.data.descriptor;
            break;
        case DataCellType::Type:
            type = other.type;
            data.type = other.data.type;
            break;
        default:
            type = DataCellType::Invalid;
            break;
    }
}

lyric_runtime::DataCell::DataCell(DataCell &&other) noexcept : DataCell()
{
    type = other.type;
    switch (other.type) {
        case DataCellType::Bool:
            data.b = other.data.b;
            break;
        case DataCellType::Int64:
            data.i64 = other.data.i64;
            break;
        case DataCellType::Float64:
            data.dbl = other.data.dbl;
            break;
        case DataCellType::Char32:
            data.chr = other.data.chr;
            break;
        case DataCellType::Bytes:
            data.bytes = other.data.bytes;
            break;
        case DataCellType::String:
            data.str = other.data.str;
            break;
        case DataCellType::Status:
            data.status = other.data.status;
            break;
        case DataCellType::Namespace:
            data.ns = other.data.ns;
            break;
        case DataCellType::Protocol:
            data.protocol = other.data.protocol;
            break;
        case DataCellType::Rest:
            data.rest = other.data.rest;
            break;
        case DataCellType::Ref:
            data.ref = other.data.ref;
            break;
        case DataCellType::Descriptor:
            data.descriptor = other.data.descriptor;
            break;
        case DataCellType::Type:
            data.type = other.data.type;
            break;
        default:
            break;
    }
    other.type = DataCellType::Invalid;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::nil()
{
    DataCell cell;
    cell.type = DataCellType::Nil;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::undef()
{
    DataCell cell;
    cell.type = DataCellType::Undef;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forDescriptor(DescriptorEntry *descriptor)
{
    TU_ASSERT (descriptor != nullptr);
    DataCell cell;
    cell.type = DataCellType::Descriptor;
    cell.data.descriptor = descriptor;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forRef(BaseRef *ref)
{
    TU_ASSERT (ref != nullptr);
    DataCell cell;
    cell.type = DataCellType::Ref;
    cell.data.ref = ref;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forBytes(BytesRef *bytes)
{
    TU_ASSERT (bytes != nullptr);
    DataCell cell;
    cell.type = DataCellType::Bytes;
    cell.data.bytes = bytes;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forStatus(StatusRef *status)
{
    TU_ASSERT (status != nullptr);
    DataCell cell;
    cell.type = DataCellType::Status;
    cell.data.status = status;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forString(StringRef *str)
{
    TU_ASSERT (str != nullptr);
    DataCell cell;
    cell.type = DataCellType::String;
    cell.data.str = str;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forType(TypeEntry *type)
{
    TU_ASSERT (type != nullptr);
    DataCell cell;
    cell.type = DataCellType::Type;
    cell.data.type = type;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forNamespace(NamespaceRef *ns)
{
    TU_ASSERT (ns != nullptr);
    DataCell cell;
    cell.type = DataCellType::Namespace;
    cell.data.ns = ns;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forProtocol(ProtocolRef *protocol)
{
    TU_ASSERT (protocol != nullptr);
    DataCell cell;
    cell.type = DataCellType::Protocol;
    cell.data.protocol = protocol;
    return cell;
}

lyric_runtime::DataCell
lyric_runtime::DataCell::forRest(RestRef *rest)
{
    TU_ASSERT (rest != nullptr);
    DataCell cell;
    cell.type = DataCellType::Rest;
    cell.data.rest = rest;
    return cell;
}

lyric_runtime::DataCell&
lyric_runtime::DataCell::operator=(const DataCell &other)
{
    switch (other.type) {
        case DataCellType::Invalid:
        case DataCellType::Nil:
        case DataCellType::Undef:
            type = other.type;
            break;
        case DataCellType::Bool:
            type = other.type;
            data.b = other.data.b;
            break;
        case DataCellType::Int64:
            type = other.type;
            data.i64 = other.data.i64;
            break;
        case DataCellType::Float64:
            type = other.type;
            data.dbl = other.data.dbl;
            break;
        case DataCellType::Char32:
            type = other.type;
            data.chr = other.data.chr;
            break;
        case DataCellType::Bytes:
            type = other.type;
            data.bytes = other.data.bytes;
            break;
        case DataCellType::String:
            type = other.type;
            data.str = other.data.str;
            break;
        case DataCellType::Status:
            type = other.type;
            data.status = other.data.status;
            break;
        case DataCellType::Namespace:
            type = other.type;
            data.ns = other.data.ns;
            break;
        case DataCellType::Protocol:
            type = other.type;
            data.protocol = other.data.protocol;
            break;
        case DataCellType::Rest:
            type = other.type;
            data.rest = other.data.rest;
            break;
        case DataCellType::Ref:
            type = other.type;
            data.ref = other.data.ref;
            break;
        case DataCellType::Descriptor:
            type = other.type;
            data.descriptor = other.data.descriptor;
            break;
        case DataCellType::Type:
            type = other.type;
            data.type = other.data.type;
            break;
        default:
            type = DataCellType::Invalid;
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
            case DataCellType::Bool:
                data.b = other.data.b;
                break;
            case DataCellType::Int64:
                data.i64 = other.data.i64;
                break;
            case DataCellType::Float64:
                data.dbl = other.data.dbl;
                break;
            case DataCellType::Char32:
                data.chr = other.data.chr;
                break;
            case DataCellType::Bytes:
                data.bytes = other.data.bytes;
                break;
            case DataCellType::String:
                data.str = other.data.str;
                break;
            case DataCellType::Status:
                data.status = other.data.status;
                break;
            case DataCellType::Namespace:
                data.ns = other.data.ns;
                break;
            case DataCellType::Protocol:
                data.protocol = other.data.protocol;
                break;
            case DataCellType::Rest:
                data.rest = other.data.rest;
                break;
            case DataCellType::Ref:
                data.ref = other.data.ref;
                break;
            case DataCellType::Descriptor:
                data.descriptor = other.data.descriptor;
                break;
            case DataCellType::Type:
                data.type = other.data.type;
                break;
            default:
                break;
        }
        other.type = DataCellType::Invalid;
    }

    return *this;
}

bool
lyric_runtime::DataCell::isValid() const
{
    switch (type) {
        case DataCellType::Bool:
        case DataCellType::Bytes:
        case DataCellType::Char32:
        case DataCellType::Descriptor:
        case DataCellType::Float64:
        case DataCellType::Int64:
        case DataCellType::Namespace:
        case DataCellType::Nil:
        case DataCellType::Protocol:
        case DataCellType::Ref:
        case DataCellType::Rest:
        case DataCellType::Status:
        case DataCellType::String:
        case DataCellType::Type:
        case DataCellType::Undef:
            return true;
        default:
            return false;
    }
}

bool
lyric_runtime::DataCell::isIntrinsic() const
{
    switch (type) {
        case DataCellType::Bool:
        case DataCellType::Char32:
        case DataCellType::Float64:
        case DataCellType::Int64:
        case DataCellType::Nil:
        case DataCellType::Undef:
            return true;
        default:
            return false;
    }
}

bool
lyric_runtime::DataCell::isDescriptor() const
{
    switch (type) {
        case DataCellType::Descriptor:
        case DataCellType::Type:
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
        case DataCellType::Bytes:
        case DataCellType::Namespace:
        case DataCellType::Protocol:
        case DataCellType::Ref:
        case DataCellType::Rest:
        case DataCellType::Status:
        case DataCellType::String:
            return true;
        default:
            break;
    }
    return false;
}

lyric_object::LinkageSection
lyric_runtime::DataCell::getLinkage() const
{
    if (type != DataCellType::Descriptor)
        return lyric_object::LinkageSection::Invalid;
    return data.descriptor->getLinkageSection();
}

std::string
lyric_runtime::DataCell::toString() const
{
    switch (type) {
        case DataCellType::Nil:
            return "nil";
        case DataCellType::Undef:
            return "undef";
        case DataCellType::Bool:
            return data.b ? "true" : "false";
        case DataCellType::Int64:
            return absl::StrCat(data.i64);
        case DataCellType::Float64:
            return absl::StrCat(data.dbl);
        case DataCellType::Char32:
            return tempo_utils::convert_to_utf8(data.chr);
        case DataCellType::Bytes:
            return data.bytes->toString();
        case DataCellType::String:
            return data.str->toString();
        case DataCellType::Status:
            return data.status->toString();
        case DataCellType::Namespace:
            return data.ns->toString();
        case DataCellType::Protocol:
            return data.protocol->toString();
        case DataCellType::Rest:
            return data.rest->toString();
        case DataCellType::Ref:
            return data.ref->toString();
        case DataCellType::Descriptor:
            return absl::Substitute("<object=$0, descriptor=$1, offset=$2>",
                data.descriptor->getSegmentIndex(),
                lyric_object::linkage_section_to_name(data.descriptor->getLinkageSection()),
                data.descriptor->getDescriptorIndex());
        case DataCellType::Type:
            return absl::Substitute("<object=$0, type=$1, offset=$2>",
                data.type->getSegmentIndex(),
                data.type->getTypeDef().toString(),
                data.type->getDescriptorIndex());
        case DataCellType::Invalid:
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
        case DataCellType::Invalid:
        case DataCellType::Nil:
        case DataCellType::Undef:
            return true;
        case DataCellType::Bool:
            return lhs.data.b == rhs.data.b;
        case DataCellType::Int64:
            return lhs.data.i64 == rhs.data.i64;
        case DataCellType::Float64:
            return lhs.data.dbl == rhs.data.dbl;
        case DataCellType::Char32:
            return lhs.data.chr == rhs.data.chr;
        case DataCellType::Bytes:
            return lhs.data.bytes == rhs.data.bytes;
        case DataCellType::String:
            return lhs.data.str == rhs.data.str;
        case DataCellType::Status:
            return lhs.data.status == rhs.data.status;
        case DataCellType::Namespace:
            return lhs.data.ns == rhs.data.ns;
        case DataCellType::Protocol:
            return lhs.data.protocol == rhs.data.protocol;
        case DataCellType::Rest:
            return lhs.data.rest == rhs.data.rest;
        case DataCellType::Ref:
            return lhs.data.ref == rhs.data.ref;
        case DataCellType::Descriptor:
            return lhs.data.descriptor->getSegmentIndex() == rhs.data.descriptor->getSegmentIndex()
                && lhs.data.descriptor->getLinkageSection() == rhs.data.descriptor->getLinkageSection()
                && lhs.data.descriptor->getDescriptorIndex() == rhs.data.descriptor->getDescriptorIndex();
        case DataCellType::Type:
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
        case DataCellType::Nil:
            std::forward<tempo_utils::LogMessage>(message) << "DataCell(nil)";
            break;
        case DataCellType::Undef:
            std::forward<tempo_utils::LogMessage>(message) << "DataCell(undef)";
            break;
        case DataCellType::Bool:
            std::forward<tempo_utils::LogMessage>(message)
                << (cell.data.b ? "DataCell(boolean=true)" : "DataCell(bool=false)");
            break;
        case DataCellType::Int64:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::StrCat("DataCell(int=", cell.data.i64, ")");
            break;
        case DataCellType::Float64:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::StrCat("DataCell(float=", cell.data.dbl, ")");
            break;
        case DataCellType::Char32:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(char=$0)",
                    tempo_utils::convert_to_utf8(cell.data.chr));
            break;
        case DataCellType::Bytes:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(bytes=$0)", cell.data.bytes->toString());
            break;
        case DataCellType::String:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(string=$0)", cell.data.str->toString());
            break;
        case DataCellType::Status:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(status=$0)", cell.data.status->toString());
            break;
        case DataCellType::Namespace:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(namespace=$0)", cell.data.ns->toString());
            break;
        case DataCellType::Protocol:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(protocol=$0)", cell.data.protocol->toString());
            break;
        case DataCellType::Rest:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(rest=$0)", cell.data.rest->toString());
            break;
        case DataCellType::Ref:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(ref=$0)", cell.data.ref->toString());
            break;
        case DataCellType::Descriptor:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, descriptor=$1, offset=$2)",
                    cell.data.descriptor->getSegmentIndex(),
                    lyric_object::linkage_section_to_name(cell.data.descriptor->getLinkageSection()),
                    cell.data.descriptor->getDescriptorIndex());
            break;
        case DataCellType::Type:
            std::forward<tempo_utils::LogMessage>(message)
                << absl::Substitute("DataCell(object=$0, type=$1, offset=$2)",
                    cell.data.type->getSegmentIndex(),
                    cell.data.type->getTypeDef().toString(),
                    cell.data.type->getDescriptorIndex());
            break;
        case DataCellType::Invalid:
        default:
            std::forward<tempo_utils::LogMessage>(message) << "DataCell(INVALID)";
            break;
    }
    return std::move(message);
}
