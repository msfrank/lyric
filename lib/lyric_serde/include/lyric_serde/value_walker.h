#ifndef LYRIC_SERDE_VALUE_WALKER_H
#define LYRIC_SERDE_VALUE_WALKER_H

#include <tempo_schema/attr.h>

#include "namespace_walker.h"
#include "serde_types.h"

namespace lyric_serde {

    class ValueWalker {

    public:
        ValueWalker();
        ValueWalker(const ValueWalker &other);

        bool isValid() const;

        tu_uint32 getIndex() const;
        ValueType getValueType() const;

        bool isNil() const;
        bool getBool() const;
        tu_int64 getInt64() const;
        double getFloat64() const;
        tu_uint64 getUInt64() const;
        tu_uint32 getUInt32() const;
        tu_uint16 getUInt16() const;
        tu_uint8 getUInt8() const;
        std::string getString() const;

        NamespaceWalker getAttrNamespace() const;
        Resource getAttrResource() const;
        ValueWalker getAttrValue() const;

        NamespaceWalker getElementNamespace() const;
        Resource getElementResource() const;
        ValueWalker getElementChild(tu_uint32 index) const;
        int numElementChildren() const;

    private:
        std::shared_ptr<const internal::PatchsetReader> m_reader;
        tu_uint32 m_index;

        ValueWalker(std::shared_ptr<const internal::PatchsetReader> reader, tu_uint32 index);
        friend class PatchsetWalker;
        friend class AppendOperationWalker;
        friend class EmitOperationWalker;
        friend class InsertOperationWalker;
        friend class UpdateOperationWalker;
        friend class ReplaceOperationWalker;
        friend class RemoveOperationWalker;
    };
}

#endif // LYRIC_SERDE_VALUE_WALKER_H
