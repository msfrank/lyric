#ifndef LYRIC_OBJECT_UNION_TYPE_WALKER_H
#define LYRIC_OBJECT_UNION_TYPE_WALKER_H

#include <lyric_common/type_def.h>

#include "object_types.h"

namespace lyric_object {

    class UnionTypeWalker {

    public:
        UnionTypeWalker();
        UnionTypeWalker(const UnionTypeWalker &other);

        bool isValid() const;

        lyric_common::TypeDef getTypeDef() const;

        std::vector<TypeWalker> getMembers() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_unionAssignable;

        UnionTypeWalker(std::shared_ptr<const internal::ObjectReader> reader, void *unionAssignable);

        friend class ObjectWalker;
        friend class TypeWalker;
    };
}

#endif // LYRIC_OBJECT_UNION_TYPE_WALKER_H
