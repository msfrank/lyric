
#include <flatbuffers/idl.h>

#include <lyric_object/generated/object_schema.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/object_types.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

lyric_object::internal::ObjectReader::ObjectReader(std::span<const tu_uint8> bytes)
    : m_bytes(bytes)
{
    m_object = lyo1::GetObject(m_bytes.data());
}

bool
lyric_object::internal::ObjectReader::isValid() const
{
    return m_object != nullptr;
}

lyo1::ObjectVersion
lyric_object::internal::ObjectReader::getABI() const
{
    if (m_object == nullptr)
        return lyo1::ObjectVersion::Unknown;
    return m_object->abi();
}

tu_uint32
lyric_object::internal::ObjectReader::getVersionMajor() const
{
    if (m_object == nullptr)
        return INVALID_ADDRESS_U32;
    return m_object->version_major();
}

tu_uint32
lyric_object::internal::ObjectReader::getVersionMinor() const
{
    if (m_object == nullptr)
        return INVALID_ADDRESS_U32;
    return m_object->version_minor();
}

tu_uint32
lyric_object::internal::ObjectReader::getVersionPatch() const
{
    if (m_object == nullptr)
        return INVALID_ADDRESS_U32;
    return m_object->version_patch();
}

std::string
lyric_object::internal::ObjectReader::getVersionString() const
{
    return absl::StrCat(
        getVersionMajor(),
        ".",
        getVersionMinor(),
        ".",
        getVersionPatch());
}

lyric_common::SymbolPath
lyric_object::internal::ObjectReader::getSymbolPath(lyo1::DescriptorSection section, tu_uint32 index) const
{
    const char *fullyQualifiedName = nullptr;

    switch (section) {
        case lyo1::DescriptorSection::Existential: {
            auto *descriptor = getExistential(index);
            fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
            break;
        }
        case lyo1::DescriptorSection::Action: {
            auto *descriptor = getAction(index);
            fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
            break;
        }
        case lyo1::DescriptorSection::Call: {
            auto *descriptor = getCall(index);
            fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
            break;
        }
        case lyo1::DescriptorSection::Class: {
            auto *descriptor = getClass(index);
            fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
            break;
        }
        case lyo1::DescriptorSection::Struct: {
            auto *descriptor = getStruct(index);
            fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
            break;
        }
        case lyo1::DescriptorSection::Concept: {
            auto *descriptor = getConcept(index);
            fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
            break;
        }
        case lyo1::DescriptorSection::Field: {
            auto *descriptor = getField(index);
            fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
            break;
        }
        case lyo1::DescriptorSection::Instance: {
            auto *descriptor = getInstance(index);
            fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
            break;
        }
        case lyo1::DescriptorSection::Enum: {
            auto *descriptor = getEnum(index);
            fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
            break;
        }
        case lyo1::DescriptorSection::Static: {
            auto *descriptor = getStatic(index);
            fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
            break;
        }
        case lyo1::DescriptorSection::Namespace: {
            auto *descriptor = getNamespace(index);
            fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
            break;
        }
        case lyo1::DescriptorSection::Type:
        default:
            break;
    }

    if (fullyQualifiedName)
        return lyric_common::SymbolPath::fromString(fullyQualifiedName);
    return lyric_common::SymbolPath();
}

const lyo1::TypeDescriptor *
lyric_object::internal::ObjectReader::getType(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->types() && index < m_object->types()->size())
        return m_object->types()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numTypes() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->types() ? m_object->types()->size() : 0;
}

const lyo1::TemplateDescriptor *
lyric_object::internal::ObjectReader::getTemplate(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->templates() && index < m_object->templates()->size())
        return m_object->templates()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numTemplates() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->templates() ? m_object->templates()->size() : 0;
}

const lyo1::ExistentialDescriptor *
lyric_object::internal::ObjectReader::getExistential(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->existentials() && index < m_object->existentials()->size())
        return m_object->existentials()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numExistentials() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->existentials() ? m_object->existentials()->size() : 0;
}

const lyo1::LiteralDescriptor *
lyric_object::internal::ObjectReader::getLiteral(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->literals() && index < m_object->literals()->size())
        return m_object->literals()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numLiterals() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->literals() ? m_object->literals()->size() : 0;
}

const lyo1::CallDescriptor *
lyric_object::internal::ObjectReader::getCall(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->calls() && index < m_object->calls()->size())
        return m_object->calls()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numCalls() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->calls() ? m_object->calls()->size() : 0;
}

const lyo1::FieldDescriptor *
lyric_object::internal::ObjectReader::getField(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->fields() && index < m_object->fields()->size())
        return m_object->fields()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numFields() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->fields() ? m_object->fields()->size() : 0;
}

const lyo1::ActionDescriptor *
lyric_object::internal::ObjectReader::getAction(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->actions() && index < m_object->actions()->size())
        return m_object->actions()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numActions() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->actions() ? m_object->actions()->size() : 0;
}

const lyo1::ConceptDescriptor *
lyric_object::internal::ObjectReader::getConcept(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->concepts() && index < m_object->concepts()->size())
        return m_object->concepts()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numConcepts() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->concepts() ? m_object->concepts()->size() : 0;
}

const lyo1::ImplDescriptor *
lyric_object::internal::ObjectReader::getImpl(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->impls() && index < m_object->impls()->size())
        return m_object->impls()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numImpls() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->impls() ? m_object->impls()->size() : 0;
}

const lyo1::StaticDescriptor *
lyric_object::internal::ObjectReader::getStatic(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->statics() && index < m_object->statics()->size())
        return m_object->statics()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numStatics() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->statics() ? m_object->statics()->size() : 0;
}

const lyo1::ClassDescriptor *
lyric_object::internal::ObjectReader::getClass(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->classes() && index < m_object->classes()->size())
        return m_object->classes()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numClasses() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->classes() ? m_object->classes()->size() : 0;
}

const lyo1::StructDescriptor *
lyric_object::internal::ObjectReader::getStruct(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->structs() && index < m_object->structs()->size())
        return m_object->structs()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numStructs() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->structs() ? m_object->structs()->size() : 0;
}

const lyo1::InstanceDescriptor *
lyric_object::internal::ObjectReader::getInstance(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->instances() && index < m_object->instances()->size())
        return m_object->instances()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numInstances() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->instances() ? m_object->instances()->size() : 0;
}

const lyo1::EnumDescriptor *
lyric_object::internal::ObjectReader::getEnum(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->enums() && index < m_object->enums()->size())
        return m_object->enums()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numEnums() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->enums() ? m_object->enums()->size() : 0;
}

const lyo1::NamespaceDescriptor *
lyric_object::internal::ObjectReader::getNamespace(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->namespaces() && index < m_object->namespaces()->size())
        return m_object->namespaces()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numNamespaces() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->namespaces() ? m_object->namespaces()->size() : 0;
}

const lyo1::SymbolDescriptor *
lyric_object::internal::ObjectReader::getSymbol(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->symbols() && index < m_object->symbols()->size())
        return m_object->symbols()->Get(index);
    return nullptr;
}

const lyo1::SymbolDescriptor *
lyric_object::internal::ObjectReader::findSymbol(const lyric_common::SymbolPath &symbolPath) const
{
    if (m_object == nullptr)
        return nullptr;
    auto fullyQualifiedName = symbolPath.toString();
    return m_object->symbols() ? m_object->symbols()->LookupByKey(fullyQualifiedName.data()) : nullptr;
}

const lyo1::SymbolDescriptor *
lyric_object::internal::ObjectReader::findSymbol(lyo1::DescriptorSection section, tu_uint32 index) const
{
    auto symbolPath = getSymbolPath(section, index);
    return findSymbol(symbolPath);
}

tu_uint32
lyric_object::internal::ObjectReader::getSymbolIndex(const lyo1::SymbolDescriptor *symbol) const
{
    if (m_object == nullptr || m_object->symbols() == nullptr)
        return INVALID_ADDRESS_U32;
    tu_uint32 index = std::distance(m_object->symbols()->Get(0), symbol);
    if (index < m_object->symbols()->size())
        return index;
    return INVALID_ADDRESS_U32;
}

tu_uint32
lyric_object::internal::ObjectReader::numSymbols() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->symbols() ? m_object->symbols()->size() : 0;
}

lyric_common::AssemblyLocation
lyric_object::internal::ObjectReader::getImportLocation(tu_uint32 index) const
{
    auto *import = getImport(index);
    if (import == nullptr)
        return lyric_common::AssemblyLocation();
    return lyric_common::AssemblyLocation::fromString(import->import_location()->c_str());
}

const lyo1::ImportDescriptor *
lyric_object::internal::ObjectReader::getImport(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->imports() && index < m_object->imports()->size())
        return m_object->imports()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numImports() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->imports() ? m_object->imports()->size() : 0;
}

const lyo1::LinkDescriptor *
lyric_object::internal::ObjectReader::getLink(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->links() && index < m_object->links()->size())
        return m_object->links()->Get(index);
    return nullptr;
}

lyric_common::SymbolUrl
lyric_object::internal::ObjectReader::getLinkUrl(tu_uint32 index) const
{
    auto *link = getLink(index);
    if (link == nullptr)
        return lyric_common::SymbolUrl();
    auto symbolPath = lyric_common::SymbolPath::fromString(link->fqsn()->c_str());
    if (!symbolPath.isValid())
        return lyric_common::SymbolUrl();
    auto importLocation = getImportLocation(link->link_import());
    if (!importLocation.isValid())
        return lyric_common::SymbolUrl();
    return lyric_common::SymbolUrl(importLocation, symbolPath);
}

tu_uint32
lyric_object::internal::ObjectReader::numLinks() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->links() ? m_object->links()->size() : 0;
}

const lyo1::PluginDescriptor *
lyric_object::internal::ObjectReader::getPlugin(tu_uint32 index) const
{
    if (m_object == nullptr)
        return nullptr;
    if (m_object->plugins() && index < m_object->plugins()->size())
        return m_object->plugins()->Get(index);
    return nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::numPlugins() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->plugins() ? m_object->plugins()->size() : 0;
}

const uint8_t *
lyric_object::internal::ObjectReader::getBytecodeData() const
{
    if (m_object == nullptr)
        return nullptr;
    return m_object->bytecode() ? m_object->bytecode()->data() : nullptr;
}

tu_uint32
lyric_object::internal::ObjectReader::getBytecodeSize() const
{
    if (m_object == nullptr)
        return 0;
    return m_object->bytecode() ? m_object->bytecode()->size() : 0;
}

std::span<const tu_uint8>
lyric_object::internal::ObjectReader::bytesView() const
{
    return m_bytes;
}

std::string
lyric_object::internal::ObjectReader::dumpJson() const
{
    flatbuffers::Parser parser;
    parser.Parse((const char *) lyric_object::schema::object::data);
    parser.opts.strict_json = true;

    std::string jsonData;
    auto *err = GenText(parser, m_bytes.data(), &jsonData);
    TU_ASSERT (err == nullptr);
    return jsonData;
}
