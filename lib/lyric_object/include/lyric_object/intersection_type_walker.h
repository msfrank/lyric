#ifndef LYRIC_OBJECT_INTERSECTION_TYPE_WALKER_H
#define LYRIC_OBJECT_INTERSECTION_TYPE_WALKER_H

#include <lyric_common/type_def.h>

#include "object_types.h"
#include "type_walker.h"

namespace lyric_object {

    class IntersectionTypeWalker {

    public:
        IntersectionTypeWalker();
        IntersectionTypeWalker(const IntersectionTypeWalker &other);

        bool isValid() const;

        lyric_common::TypeDef getTypeDef() const;

        std::vector<TypeWalker> getMembers() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_intersectionAssignable;

        IntersectionTypeWalker(std::shared_ptr<const internal::ObjectReader> reader, void *intersectionAssignable);

        friend class ObjectWalker;
        friend class TypeWalker;
    };
}

#endif // LYRIC_OBJECT_INTERSECTION_TYPE_WALKER_H
