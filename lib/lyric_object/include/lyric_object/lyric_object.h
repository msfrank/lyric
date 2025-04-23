#ifndef LYRIC_OBJECT_LYRIC_OBJECT_H
#define LYRIC_OBJECT_LYRIC_OBJECT_H

#include <span>

#include <lyric_common/symbol_url.h>
#include <lyric_object/object_walker.h>
#include <lyric_object/object_types.h>
#include <tempo_utils/immutable_bytes.h>

namespace lyric_object {

    class LyricObject {

    public:
        LyricObject();
        LyricObject(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes);
        LyricObject(std::span<const tu_uint8> unownedBytes);
        LyricObject(const LyricObject &other);

        bool isValid() const;

        ObjectVersion getABI() const;

        ObjectWalker getObject() const;

        const uint8_t *getBytecodeData() const;
        uint32_t getBytecodeSize() const;

        std::shared_ptr<const internal::ObjectReader> getReader() const;
        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

        static bool verify(std::span<const tu_uint8> bytes);

    private:
        std::shared_ptr<const tempo_utils::ImmutableBytes> m_bytes;
        std::shared_ptr<const internal::ObjectReader> m_reader;
    };
}

#endif // LYRIC_OBJECT_LYRIC_OBJECT_H
