#ifndef LYRIC_OBJECT_LINK_WALKER_H
#define LYRIC_OBJECT_LINK_WALKER_H

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class ImportWalker;

    class LinkWalker {

    public:
        LinkWalker();
        LinkWalker(const LinkWalker &other);

        bool isValid() const;

        lyric_common::SymbolPath getSymbolPath() const;

        LinkageSection getLinkageSection() const;
        tu_uint32 getLinkageIndex() const;

        ImportWalker getLinkImport() const;
        lyric_common::SymbolUrl getLinkUrl(const lyric_common::ModuleLocation &base = {}) const;
        tu_uint32 getLinkAddress() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_linkOffset;

        LinkWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 linkOffset);

        friend class ActionWalker;
        friend class BindingWalker;
        friend class CallWalker;
        friend class ClassAction;
        friend class ClassMember;
        friend class ClassMethod;
        friend class ClassWalker;
        friend class ConceptAction;
        friend class ConceptWalker;
        friend class EnumMember;
        friend class EnumMethod;
        friend class EnumWalker;
        friend class ExistentialMethod;
        friend class ExistentialWalker;
        friend class ExtensionWalker;
        friend class FieldWalker;
        friend class ImplWalker;
        friend class InstanceMember;
        friend class InstanceMethod;
        friend class InstanceWalker;
        friend class NamespaceWalker;
        friend class ObjectWalker;
        friend class ParameterWalker;
        friend class PlaceholderTypeWalker;
        friend class StaticWalker;
        friend class StructMember;
        friend class StructMethod;
        friend class StructWalker;
    };
}

#endif // LYRIC_OBJECT_LINK_WALKER_H
