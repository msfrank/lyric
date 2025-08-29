#ifndef LYRIC_OBJECT_FIELD_WALKER_H
#define LYRIC_OBJECT_FIELD_WALKER_H

#include <lyric_common/symbol_path.h>

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class CallWalker;
    class LinkWalker;

    class FieldWalker {

    public:
        FieldWalker();
        FieldWalker(const FieldWalker &other);

        bool isValid() const;

        bool isVariable() const;
        bool isDeclOnly() const;
        AccessType getAccess() const;

        lyric_common::SymbolPath getSymbolPath() const;

        TypeWalker getFieldType() const;

        bool hasInitializer() const;
        AddressType initializerAddressType() const;
        CallWalker getNearInitializer() const;
        LinkWalker getFarInitializer() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_fieldOffset;

        FieldWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 fieldOffset);

        friend class ClassWalker;
        friend class EnumMember;
        friend class InstanceMember;
        friend class ObjectWalker;
        friend class StructMember;
    };
}

#endif // LYRIC_OBJECT_FIELD_WALKER_H
