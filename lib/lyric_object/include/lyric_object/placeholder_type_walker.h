#ifndef LYRIC_OBJECT_PLACEHOLDER_TYPE_WALKER_H
#define LYRIC_OBJECT_PLACEHOLDER_TYPE_WALKER_H

#include <lyric_common/type_def.h>

#include "object_types.h"
#include "type_walker.h"

namespace lyric_object {

    // forward declarations
    class TemplateWalker;
    class LinkWalker;

    class PlaceholderTypeWalker {

    public:
        PlaceholderTypeWalker();
        PlaceholderTypeWalker(const PlaceholderTypeWalker &other);

        bool isValid() const;

        lyric_common::TypeDef getTypeDef() const;

        tu_uint8 getPlaceholderIndex() const;

        AddressType placeholderTemplateAddressType() const;
        TemplateWalker getNearPlaceholderTemplate() const;
        LinkWalker getFarPlaceholderTemplate() const;

        std::vector<TypeWalker> getParameters() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_placeholderAssignable;

        PlaceholderTypeWalker(std::shared_ptr<const internal::ObjectReader> reader, void *placeholderAssignable);

        friend class ObjectWalker;
        friend class TypeWalker;
    };
}

#endif // LYRIC_OBJECT_PLACEHOLDER_TYPE_WALKER_H
