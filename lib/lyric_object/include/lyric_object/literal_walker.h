#ifndef LYRIC_OBJECT_LITERAL_WALKER_H
#define LYRIC_OBJECT_LITERAL_WALKER_H

#include "object_types.h"

namespace lyric_object {

    class LiteralWalker {

    public:
        LiteralWalker();
        LiteralWalker(const LiteralWalker &other);

        bool isValid() const;

        ValueType getValueType() const;

        bool boolValue() const;
        char32_t charValue() const;
        tu_int64 int64Value() const;
        double float64Value() const;
        std::string_view stringValue() const;
        std::span<const tu_uint8> bytesValue() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_literalOffset;

        LiteralWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 literalOffset);

        friend class ObjectWalker;
    };
}

#endif // LYRIC_OBJECT_LITERAL_WALKER_H
