#ifndef LYRIC_OBJECT_CONCRETE_TYPE_WALKER_H
#define LYRIC_OBJECT_CONCRETE_TYPE_WALKER_H

#include <lyric_common/type_def.h>

#include "object_types.h"
#include "type_walker.h"

namespace lyric_object {

    class ConcreteTypeWalker {

    public:
        ConcreteTypeWalker();
        ConcreteTypeWalker(const ConcreteTypeWalker &other);

        bool isValid() const;

        lyric_common::TypeDef getTypeDef() const;

        LinkageSection getLinkageSection() const;
        tu_uint32 getLinkageIndex() const;

        std::vector<TypeWalker> getParameters() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_concreteAssignable;

        ConcreteTypeWalker(std::shared_ptr<const internal::ObjectReader> reader, void *concreteAssignable);

        friend class ObjectWalker;
        friend class TypeWalker;
    };
}

#endif // LYRIC_OBJECT_CONCRETE_TYPE_WALKER_H
