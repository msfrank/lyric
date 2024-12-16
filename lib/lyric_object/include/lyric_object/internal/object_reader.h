#ifndef LYRIC_OBJECT_INTERNAL_OBJECT_READER_H
#define LYRIC_OBJECT_INTERNAL_OBJECT_READER_H

#include <span>

#include <lyric_common/symbol_url.h>
#include <lyric_object/generated/object.h>
#include <lyric_object/object_types.h>

namespace lyric_object::internal {

    class ObjectReader {

    public:
        ObjectReader(std::span<const tu_uint8> bytes);

        bool isValid() const;

        lyo1::ObjectVersion getABI() const;
        tu_uint32 getVersionMajor() const;
        tu_uint32 getVersionMinor() const;
        tu_uint32 getVersionPatch() const;
        std::string getVersionString() const;

        lyric_common::SymbolPath getSymbolPath(lyo1::DescriptorSection section, tu_uint32 index) const;

        const lyo1::TypeDescriptor *getType(tu_uint32 index) const;
        tu_uint32 numTypes() const;

        const lyo1::TemplateDescriptor *getTemplate(tu_uint32 index) const;
        tu_uint32 numTemplates() const;

        const lyo1::ExistentialDescriptor *getExistential(tu_uint32 index) const;
        tu_uint32 numExistentials() const;

        const lyo1::LiteralDescriptor *getLiteral(tu_uint32 index) const;
        tu_uint32 numLiterals() const;

        const lyo1::CallDescriptor *getCall(tu_uint32 index) const;
        tu_uint32 numCalls() const;

        const lyo1::FieldDescriptor *getField(tu_uint32 index) const;
        tu_uint32 numFields() const;

        const lyo1::ActionDescriptor *getAction(tu_uint32 index) const;
        tu_uint32 numActions() const;

        const lyo1::ConceptDescriptor *getConcept(tu_uint32 index) const;
        tu_uint32 numConcepts() const;

        const lyo1::ImplDescriptor *getImpl(tu_uint32 index) const;
        tu_uint32 numImpls() const;

        const lyo1::StaticDescriptor *getStatic(tu_uint32 index) const;
        tu_uint32 numStatics() const;

        const lyo1::ClassDescriptor *getClass(tu_uint32 index) const;
        tu_uint32 numClasses() const;

        const lyo1::StructDescriptor *getStruct(tu_uint32 index) const;
        tu_uint32 numStructs() const;

        const lyo1::InstanceDescriptor *getInstance(tu_uint32 index) const;
        tu_uint32 numInstances() const;

        const lyo1::EnumDescriptor *getEnum(tu_uint32 index) const;
        tu_uint32 numEnums() const;

        const lyo1::NamespaceDescriptor *getNamespace(tu_uint32 index) const;
        tu_uint32 numNamespaces() const;

        const lyo1::BindingDescriptor *getBinding(tu_uint32 index) const;
        tu_uint32 numBindings() const;

        const lyo1::SymbolDescriptor *getSymbol(tu_uint32 index) const;
        tu_uint32 getSymbolIndex(const lyric_common::SymbolPath &symbolPath) const;
        const lyo1::SymbolDescriptor *findSymbol(const lyric_common::SymbolPath &symbolPath) const;
        const lyo1::SymbolDescriptor *findSymbol(lyo1::DescriptorSection section, tu_uint32 index) const;
        tu_uint32 numSymbols() const;

        lyric_common::ModuleLocation getImportLocation(tu_uint32 index) const;
        const lyo1::ImportDescriptor *getImport(tu_uint32 index) const;
        tu_uint32 numImports() const;

        lyric_common::SymbolUrl getLinkUrl(tu_uint32 index) const;
        const lyo1::LinkDescriptor *getLink(tu_uint32 index) const;
        tu_uint32 numLinks() const;

        const lyo1::PluginDescriptor *getPlugin(tu_uint32 index) const;
        tu_uint32 numPlugins() const;

        const uint8_t *getBytecodeData() const;
        tu_uint32 getBytecodeSize() const;

        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

    private:
        std::span<const tu_uint8> m_bytes;
        const lyo1::Object *m_object;
    };

    inline lyric_object::LinkageSection
    descriptor_to_linkage_section(lyo1::DescriptorSection section)
    {
        switch (section) {
            case lyo1::DescriptorSection::Action:
                return lyric_object::LinkageSection::Action;
            case lyo1::DescriptorSection::Call:
                return lyric_object::LinkageSection::Call;
            case lyo1::DescriptorSection::Class:
                return lyric_object::LinkageSection::Class;
            case lyo1::DescriptorSection::Concept:
                return lyric_object::LinkageSection::Concept;
            case lyo1::DescriptorSection::Enum:
                return lyric_object::LinkageSection::Enum;
            case lyo1::DescriptorSection::Existential:
                return lyric_object::LinkageSection::Existential;
            case lyo1::DescriptorSection::Field:
                return lyric_object::LinkageSection::Field;
            case lyo1::DescriptorSection::Instance:
                return lyric_object::LinkageSection::Instance;
            case lyo1::DescriptorSection::Literal:
                return lyric_object::LinkageSection::Literal;
            case lyo1::DescriptorSection::Namespace:
                return lyric_object::LinkageSection::Namespace;
            case lyo1::DescriptorSection::Static:
                return lyric_object::LinkageSection::Static;
            case lyo1::DescriptorSection::Struct:
                return lyric_object::LinkageSection::Struct;
            case lyo1::DescriptorSection::Type:
                return lyric_object::LinkageSection::Type;
            case lyo1::DescriptorSection::Invalid:
            default:
                return lyric_object::LinkageSection::Invalid;
        }
    }

    inline lyo1::DescriptorSection
    linkage_to_descriptor_section(lyric_object::LinkageSection section)
    {
        switch (section) {
            case LinkageSection::Action:
                return lyo1::DescriptorSection::Action;
            case LinkageSection::Call:
                return lyo1::DescriptorSection::Call;
            case LinkageSection::Class:
                return lyo1::DescriptorSection::Class;
            case LinkageSection::Concept:
                return lyo1::DescriptorSection::Concept;
            case LinkageSection::Enum:
                return lyo1::DescriptorSection::Enum;
            case LinkageSection::Existential:
                return lyo1::DescriptorSection::Existential;
            case LinkageSection::Field:
                return lyo1::DescriptorSection::Field;
            case LinkageSection::Instance:
                return lyo1::DescriptorSection::Instance;
            case LinkageSection::Literal:
                return lyo1::DescriptorSection::Literal;
            case LinkageSection::Namespace:
                return lyo1::DescriptorSection::Namespace;
            case LinkageSection::Static:
                return lyo1::DescriptorSection::Static;
            case LinkageSection::Struct:
                return lyo1::DescriptorSection::Struct;
            case LinkageSection::Type:
                return lyo1::DescriptorSection::Type;
            case LinkageSection::Invalid:
            default:
                return lyo1::DescriptorSection::Invalid;
        }
    }
}

#endif // LYRIC_OBJECT_INTERNAL_OBJECT_READER_H
