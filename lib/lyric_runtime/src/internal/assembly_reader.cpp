//
// #include <flatbuffers/idl.h>
//
// #include <lyric_runtime/generated/assembly_schema.h>
// #include <lyric_runtime/internal/assembly_reader.h>
// #include <lyric_runtime/literal_cell.h>
// #include <lyric_runtime/runtime_types.h>
// #include <tempo_utils/big_endian.h>
// #include <tempo_utils/log_stream.h>
//
// lyric_runtime::internal::AssemblyReader::AssemblyReader(std::span<const tu_uint8> bytes)
//     : m_bytes(bytes)
// {
//     m_assembly = lya1::GetAssembly(m_bytes.data());
// }
//
// bool
// lyric_runtime::internal::AssemblyReader::isValid() const
// {
//     return m_assembly != nullptr;
// }
//
// lya1::AssemblyVersion
// lyric_runtime::internal::AssemblyReader::getABI() const
// {
//     if (m_assembly == nullptr)
//         return lya1::AssemblyVersion::Unknown;
//     return m_assembly->abi();
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::getVersionMajor() const
// {
//     if (m_assembly == nullptr)
//         return INVALID_ADDRESS_U32;
//     return m_assembly->version_major();
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::getVersionMinor() const
// {
//     if (m_assembly == nullptr)
//         return INVALID_ADDRESS_U32;
//     return m_assembly->version_minor();
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::getVersionPatch() const
// {
//     if (m_assembly == nullptr)
//         return INVALID_ADDRESS_U32;
//     return m_assembly->version_patch();
// }
//
// std::string
// lyric_runtime::internal::AssemblyReader::getVersionString() const
// {
//     return absl::StrCat(
//         getVersionMajor(),
//         ".",
//         getVersionMinor(),
//         ".",
//         getVersionPatch());
// }
//
// lyric_common::SymbolPath
// lyric_runtime::internal::AssemblyReader::getSymbolPath(lya1::DescriptorSection section, tu_uint32 index) const
// {
//     const char *fullyQualifiedName = nullptr;
//
//     switch (section) {
//         case lya1::DescriptorSection::Existential: {
//             auto *descriptor = getExistential(index);
//             fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
//             break;
//         }
//         case lya1::DescriptorSection::Action: {
//             auto *descriptor = getAction(index);
//             fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
//             break;
//         }
//         case lya1::DescriptorSection::Call: {
//             auto *descriptor = getCall(index);
//             fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
//             break;
//         }
//         case lya1::DescriptorSection::Class: {
//             auto *descriptor = getClass(index);
//             fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
//             break;
//         }
//         case lya1::DescriptorSection::Struct: {
//             auto *descriptor = getStruct(index);
//             fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
//             break;
//         }
//         case lya1::DescriptorSection::Concept: {
//             auto *descriptor = getConcept(index);
//             fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
//             break;
//         }
//         case lya1::DescriptorSection::Field: {
//             auto *descriptor = getField(index);
//             fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
//             break;
//         }
//         case lya1::DescriptorSection::Instance: {
//             auto *descriptor = getInstance(index);
//             fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
//             break;
//         }
//         case lya1::DescriptorSection::Enum: {
//             auto *descriptor = getEnum(index);
//             fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
//             break;
//         }
//         case lya1::DescriptorSection::Static: {
//             auto *descriptor = getStatic(index);
//             fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
//             break;
//         }
//         case lya1::DescriptorSection::Namespace: {
//             auto *descriptor = getNamespace(index);
//             fullyQualifiedName = descriptor? descriptor->fqsn()->c_str() : nullptr;
//             break;
//         }
//         case lya1::DescriptorSection::Type:
//         default:
//             break;
//     }
//
//     if (fullyQualifiedName)
//         return lyric_common::SymbolPath::fromString(fullyQualifiedName);
//     return lyric_common::SymbolPath();
// }
//
// const lya1::TypeDescriptor *
// lyric_runtime::internal::AssemblyReader::getType(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->types() && index < m_assembly->types()->size())
//         return m_assembly->types()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numTypes() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->types() ? m_assembly->types()->size() : 0;
// }
//
// const lya1::TemplateDescriptor *
// lyric_runtime::internal::AssemblyReader::getTemplate(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->templates() && index < m_assembly->templates()->size())
//         return m_assembly->templates()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numTemplates() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->templates() ? m_assembly->templates()->size() : 0;
// }
//
// const lya1::ExistentialDescriptor *
// lyric_runtime::internal::AssemblyReader::getExistential(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->existentials() && index < m_assembly->existentials()->size())
//         return m_assembly->existentials()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numExistentials() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->existentials() ? m_assembly->existentials()->size() : 0;
// }
//
// const lya1::LiteralDescriptor *
// lyric_runtime::internal::AssemblyReader::getLiteral(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->literals() && index < m_assembly->literals()->size())
//         return m_assembly->literals()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numLiterals() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->literals() ? m_assembly->literals()->size() : 0;
// }
//
// const lya1::CallDescriptor *
// lyric_runtime::internal::AssemblyReader::getCall(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->calls() && index < m_assembly->calls()->size())
//         return m_assembly->calls()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numCalls() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->calls() ? m_assembly->calls()->size() : 0;
// }
//
// const lya1::FieldDescriptor *
// lyric_runtime::internal::AssemblyReader::getField(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->fields() && index < m_assembly->fields()->size())
//         return m_assembly->fields()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numFields() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->fields() ? m_assembly->fields()->size() : 0;
// }
//
// const lya1::ActionDescriptor *
// lyric_runtime::internal::AssemblyReader::getAction(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->actions() && index < m_assembly->actions()->size())
//         return m_assembly->actions()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numActions() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->actions() ? m_assembly->actions()->size() : 0;
// }
//
// const lya1::ConceptDescriptor *
// lyric_runtime::internal::AssemblyReader::getConcept(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->concepts() && index < m_assembly->concepts()->size())
//         return m_assembly->concepts()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numConcepts() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->concepts() ? m_assembly->concepts()->size() : 0;
// }
//
// const lya1::StaticDescriptor *
// lyric_runtime::internal::AssemblyReader::getStatic(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->statics() && index < m_assembly->statics()->size())
//         return m_assembly->statics()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numStatics() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->statics() ? m_assembly->statics()->size() : 0;
// }
//
// const lya1::ClassDescriptor *
// lyric_runtime::internal::AssemblyReader::getClass(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->classes() && index < m_assembly->classes()->size())
//         return m_assembly->classes()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numClasses() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->classes() ? m_assembly->classes()->size() : 0;
// }
//
// const lya1::StructDescriptor *
// lyric_runtime::internal::AssemblyReader::getStruct(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->structs() && index < m_assembly->structs()->size())
//         return m_assembly->structs()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numStructs() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->structs() ? m_assembly->structs()->size() : 0;
// }
//
// const lya1::InstanceDescriptor *
// lyric_runtime::internal::AssemblyReader::getInstance(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->instances() && index < m_assembly->instances()->size())
//         return m_assembly->instances()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numInstances() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->instances() ? m_assembly->instances()->size() : 0;
// }
//
// const lya1::EnumDescriptor *
// lyric_runtime::internal::AssemblyReader::getEnum(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->enums() && index < m_assembly->enums()->size())
//         return m_assembly->enums()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numEnums() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->enums() ? m_assembly->enums()->size() : 0;
// }
//
// const lya1::NamespaceDescriptor *
// lyric_runtime::internal::AssemblyReader::getNamespace(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->namespaces() && index < m_assembly->namespaces()->size())
//         return m_assembly->namespaces()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numNamespaces() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->namespaces() ? m_assembly->namespaces()->size() : 0;
// }
//
// const lya1::SymbolDescriptor *
// lyric_runtime::internal::AssemblyReader::getSymbol(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->symbols() && index < m_assembly->symbols()->size())
//         return m_assembly->symbols()->Get(index);
//     return nullptr;
// }
//
// const lya1::SymbolDescriptor *
// lyric_runtime::internal::AssemblyReader::findSymbol(const lyric_common::SymbolPath &symbolPath) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     auto fullyQualifiedName = symbolPath.toString();
//     return m_assembly->symbols() ? m_assembly->symbols()->LookupByKey(fullyQualifiedName.data()) : nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numSymbols() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->symbols() ? m_assembly->symbols()->size() : 0;
// }
//
// lyric_common::AssemblyLocation
// lyric_runtime::internal::AssemblyReader::getImportLocation(tu_uint32 index) const
// {
//     auto *import = getImport(index);
//     if (import == nullptr)
//         return lyric_common::AssemblyLocation();
//     return lyric_common::AssemblyLocation::fromString(import->import_location()->c_str());
// }
//
// const lya1::ImportDescriptor *
// lyric_runtime::internal::AssemblyReader::getImport(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->imports() && index < m_assembly->imports()->size())
//         return m_assembly->imports()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numImports() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->imports() ? m_assembly->imports()->size() : 0;
// }
//
// const lya1::LinkDescriptor *
// lyric_runtime::internal::AssemblyReader::getLink(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->links() && index < m_assembly->links()->size())
//         return m_assembly->links()->Get(index);
//     return nullptr;
// }
//
// lyric_common::SymbolUrl
// lyric_runtime::internal::AssemblyReader::getLinkUrl(tu_uint32 index) const
// {
//     auto *link = getLink(index);
//     if (link == nullptr)
//         return lyric_common::SymbolUrl();
//     auto symbolPath = lyric_common::SymbolPath::fromString(link->fqsn()->c_str());
//     if (!symbolPath.isValid())
//         return lyric_common::SymbolUrl();
//     auto importLocation = getImportLocation(link->link_import());
//     if (!importLocation.isValid())
//         return lyric_common::SymbolUrl();
//     return lyric_common::SymbolUrl(importLocation, symbolPath);
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numLinks() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->links() ? m_assembly->links()->size() : 0;
// }
//
// const lya1::PluginDescriptor *
// lyric_runtime::internal::AssemblyReader::getPlugin(tu_uint32 index) const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     if (m_assembly->plugins() && index < m_assembly->plugins()->size())
//         return m_assembly->plugins()->Get(index);
//     return nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::numPlugins() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->plugins() ? m_assembly->plugins()->size() : 0;
// }
//
// const uint8_t *
// lyric_runtime::internal::AssemblyReader::getBytecodeData() const
// {
//     if (m_assembly == nullptr)
//         return nullptr;
//     return m_assembly->bytecode() ? m_assembly->bytecode()->data() : nullptr;
// }
//
// tu_uint32
// lyric_runtime::internal::AssemblyReader::getBytecodeSize() const
// {
//     if (m_assembly == nullptr)
//         return 0;
//     return m_assembly->bytecode() ? m_assembly->bytecode()->size() : 0;
// }
//
// std::span<const tu_uint8>
// lyric_runtime::internal::AssemblyReader::bytesView() const
// {
//     return m_bytes;
// }
//
// std::string
// lyric_runtime::internal::AssemblyReader::dumpJson() const
// {
//     flatbuffers::Parser parser;
//     parser.Parse((const char *) lyric_runtime::schema::assembly::data);
//     parser.opts.strict_json = true;
//
//     std::string jsonData;
//     auto *err = GenText(parser, m_bytes.data(), &jsonData);
//     TU_ASSERT (err == nullptr);
//     return jsonData;
// }
