
#include <flatbuffers/verifier.h>

#include <lyric_object/internal/object_reader.h>
#include <lyric_object/lyric_object.h>
#include <lyric_object/object_types.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

lyric_object::LyricObject::LyricObject()
{
}

lyric_object::LyricObject::LyricObject(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes)
    : m_bytes(immutableBytes)
{
    TU_ASSERT (m_bytes != nullptr);
    std::span<const tu_uint8> bytes(m_bytes->getData(), m_bytes->getSize());
    m_reader = std::make_shared<internal::ObjectReader>(bytes);
}

lyric_object::LyricObject::LyricObject(std::span<const tu_uint8> unownedBytes)
{
    m_reader = std::make_shared<const internal::ObjectReader>(unownedBytes);
}

lyric_object::LyricObject::LyricObject(const LyricObject &other)
    : m_bytes(other.m_bytes),
      m_reader(other.m_reader)
{
}

bool
lyric_object::LyricObject::isValid() const
{
    if (m_reader == nullptr)
        return false;
    return m_reader->isValid();
}

lyric_object::ObjectVersion
lyric_object::LyricObject::getABI() const
{
    if (m_reader == nullptr)
        return ObjectVersion::Unknown;
    switch (m_reader->getABI()) {
        case lyo1::ObjectVersion::Version1:
            return ObjectVersion::Version1;
        case lyo1::ObjectVersion::Unknown:
        default:
            return ObjectVersion::Unknown;
    }
}

lyric_object::ObjectWalker
lyric_object::LyricObject::getObject() const
{
    if (!isValid())
        return {};
    return ObjectWalker(m_reader);
}

const uint8_t *
lyric_object::LyricObject::getBytecodeData() const
{
    if (m_reader == nullptr)
        return nullptr;
    return m_reader->getBytecodeData();
}

uint32_t
lyric_object::LyricObject::getBytecodeSize() const
{
    if (m_reader == nullptr)
        return 0;
    return m_reader->getBytecodeSize();
}

std::shared_ptr<const lyric_object::internal::ObjectReader>
lyric_object::LyricObject::getReader() const
{
    return m_reader;
}

std::span<const tu_uint8>
lyric_object::LyricObject::bytesView() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->bytesView();
}

bool
lyric_object::LyricObject::verify(std::span<const tu_uint8> bytes)
{
    flatbuffers::Verifier verifier(bytes.data(), bytes.size());
    return lyo1::VerifyObjectBuffer(verifier);
}
