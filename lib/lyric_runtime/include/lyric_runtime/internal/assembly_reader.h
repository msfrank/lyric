// #ifndef LYRIC_RUNTIME_ASSEMBLY_READER_H
// #define LYRIC_RUNTIME_ASSEMBLY_READER_H
//
// #include <span>
//
// #include <lyric_common/symbol_url.h>
// #include <lyric_runtime/generated/assembly.h>
// #include <lyric_runtime/runtime_types.h>
//
// namespace lyric_runtime::internal {
//
//     class AssemblyReader {
//
//     public:
//         AssemblyReader(std::span<const tu_uint8> bytes);
//
//         bool isValid() const;
//
//         lya1::AssemblyVersion getABI() const;
//         tu_uint32 getVersionMajor() const;
//         tu_uint32 getVersionMinor() const;
//         tu_uint32 getVersionPatch() const;
//         std::string getVersionString() const;
//
//         lyric_common::SymbolPath getSymbolPath(lya1::DescriptorSection section, tu_uint32 index) const;
//
//         const lya1::TypeDescriptor *getType(tu_uint32 index) const;
//         tu_uint32 numTypes() const;
//
//         const lya1::TemplateDescriptor *getTemplate(tu_uint32 index) const;
//         tu_uint32 numTemplates() const;
//
//         const lya1::ExistentialDescriptor *getExistential(tu_uint32 index) const;
//         tu_uint32 numExistentials() const;
//
//         const lya1::LiteralDescriptor *getLiteral(tu_uint32 index) const;
//         tu_uint32 numLiterals() const;
//
//         const lya1::CallDescriptor *getCall(tu_uint32 index) const;
//         tu_uint32 numCalls() const;
//
//         const lya1::FieldDescriptor *getField(tu_uint32 index) const;
//         tu_uint32 numFields() const;
//
//         const lya1::ActionDescriptor *getAction(tu_uint32 index) const;
//         tu_uint32 numActions() const;
//
//         const lya1::ConceptDescriptor *getConcept(tu_uint32 index) const;
//         tu_uint32 numConcepts() const;
//
//         const lya1::StaticDescriptor *getStatic(tu_uint32 index) const;
//         tu_uint32 numStatics() const;
//
//         const lya1::ClassDescriptor *getClass(tu_uint32 index) const;
//         tu_uint32 numClasses() const;
//
//         const lya1::StructDescriptor *getStruct(tu_uint32 index) const;
//         tu_uint32 numStructs() const;
//
//         const lya1::InstanceDescriptor *getInstance(tu_uint32 index) const;
//         tu_uint32 numInstances() const;
//
//         const lya1::EnumDescriptor *getEnum(tu_uint32 index) const;
//         tu_uint32 numEnums() const;
//
//         const lya1::NamespaceDescriptor *getNamespace(tu_uint32 index) const;
//         tu_uint32 numNamespaces() const;
//
//         const lya1::SymbolDescriptor *getSymbol(tu_uint32 index) const;
//         const lya1::SymbolDescriptor *findSymbol(const lyric_common::SymbolPath &symbolPath) const;
//         tu_uint32 numSymbols() const;
//
//         lyric_common::AssemblyLocation getImportLocation(tu_uint32 index) const;
//         const lya1::ImportDescriptor *getImport(tu_uint32 index) const;
//         tu_uint32 numImports() const;
//
//         lyric_common::SymbolUrl getLinkUrl(tu_uint32 index) const;
//         const lya1::LinkDescriptor *getLink(tu_uint32 index) const;
//         tu_uint32 numLinks() const;
//
//         const lya1::PluginDescriptor *getPlugin(tu_uint32 index) const;
//         tu_uint32 numPlugins() const;
//
//         const uint8_t *getBytecodeData() const;
//         tu_uint32 getBytecodeSize() const;
//
//         std::span<const tu_uint8> bytesView() const;
//
//         std::string dumpJson() const;
//
//     private:
//         std::span<const tu_uint8> m_bytes;
//         const lya1::Assembly *m_assembly;
//     };
//
//     inline lyric_runtime::LinkageSection
//     descriptor_to_linkage_section(lya1::DescriptorSection section)
//     {
//         switch (section) {
//             case lya1::DescriptorSection::Action:
//                 return lyric_runtime::LinkageSection::Action;
//             case lya1::DescriptorSection::Call:
//                 return lyric_runtime::LinkageSection::Call;
//             case lya1::DescriptorSection::Class:
//                 return lyric_runtime::LinkageSection::Class;
//             case lya1::DescriptorSection::Concept:
//                 return lyric_runtime::LinkageSection::Concept;
//             case lya1::DescriptorSection::Enum:
//                 return lyric_runtime::LinkageSection::Enum;
//             case lya1::DescriptorSection::Existential:
//                 return lyric_runtime::LinkageSection::Existential;
//             case lya1::DescriptorSection::Field:
//                 return lyric_runtime::LinkageSection::Field;
//             case lya1::DescriptorSection::Generic:
//                 return lyric_runtime::LinkageSection::Generic;
//             case lya1::DescriptorSection::Instance:
//                 return lyric_runtime::LinkageSection::Instance;
//             case lya1::DescriptorSection::Literal:
//                 return lyric_runtime::LinkageSection::Literal;
//             case lya1::DescriptorSection::Namespace:
//                 return lyric_runtime::LinkageSection::Namespace;
//             case lya1::DescriptorSection::Static:
//                 return lyric_runtime::LinkageSection::Static;
//             case lya1::DescriptorSection::Struct:
//                 return lyric_runtime::LinkageSection::Struct;
//             case lya1::DescriptorSection::Type:
//                 return lyric_runtime::LinkageSection::Type;
//             case lya1::DescriptorSection::Invalid:
//             default:
//                 return lyric_runtime::LinkageSection::Invalid;
//         }
//     }
// }
//
// #endif // LYRIC_RUNTIME_ASSEMBLY_READER_H
