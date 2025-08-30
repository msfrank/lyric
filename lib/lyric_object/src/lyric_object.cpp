
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

lyric_object::ActionWalker
lyric_object::LyricObject::getAction(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return ActionWalker(m_reader, index);
}

int
lyric_object::LyricObject::numActions() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numActions();
}

lyric_object::BindingWalker
lyric_object::LyricObject::getBinding(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return BindingWalker(m_reader, index);
}

int
lyric_object::LyricObject::numBindings() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numBindings();
}

lyric_object::CallWalker
lyric_object::LyricObject::getCall(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return CallWalker(m_reader, index);
}

int
lyric_object::LyricObject::numCalls() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numCalls();
}

lyric_object::ClassWalker
lyric_object::LyricObject::getClass(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return ClassWalker(m_reader, index);
}

int
lyric_object::LyricObject::numClasses() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numClasses();
}

lyric_object::ConceptWalker
lyric_object::LyricObject::getConcept(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return ConceptWalker(m_reader, index);
}

int
lyric_object::LyricObject::numConcepts() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numConcepts();
}

lyric_object::EnumWalker
lyric_object::LyricObject::getEnum(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return EnumWalker(m_reader, index);
}

int
lyric_object::LyricObject::numEnums() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numEnums();
}

lyric_object::ExistentialWalker
lyric_object::LyricObject::getExistential(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return ExistentialWalker(m_reader, index);
}

int
lyric_object::LyricObject::numExistentials() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numExistentials();
}

lyric_object::FieldWalker
lyric_object::LyricObject::getField(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return FieldWalker(m_reader, index);
}

int
lyric_object::LyricObject::numFields() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numFields();
}

lyric_object::ImplWalker
lyric_object::LyricObject::getImpl(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return ImplWalker(m_reader, index);
}

int
lyric_object::LyricObject::numImpls() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numImpls();
}

lyric_object::ImportWalker
lyric_object::LyricObject::getImport(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return ImportWalker(m_reader, index);
}

int
lyric_object::LyricObject::numImports() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numImports();
}

lyric_object::InstanceWalker
lyric_object::LyricObject::getInstance(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return InstanceWalker(m_reader, index);
}

int
lyric_object::LyricObject::numInstances() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numInstances();
}

lyric_object::LinkWalker
lyric_object::LyricObject::getLink(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return LinkWalker(m_reader, index);
}

int
lyric_object::LyricObject::numLinks() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numLinks();
}

lyric_object::LiteralWalker
lyric_object::LyricObject::getLiteral(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return LiteralWalker(m_reader, index);
}

int
lyric_object::LyricObject::numLiterals() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numLiterals();
}

lyric_object::NamespaceWalker
lyric_object::LyricObject::getNamespace(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return NamespaceWalker(m_reader, index);
}

int
lyric_object::LyricObject::numNamespaces() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numNamespaces();
}

lyric_object::StaticWalker
lyric_object::LyricObject::getStatic(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return StaticWalker(m_reader, index);
}

int
lyric_object::LyricObject::numStatics() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numStatics();
}

lyric_object::StructWalker
lyric_object::LyricObject::getStruct(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return StructWalker(m_reader, index);
}

int
lyric_object::LyricObject::numStructs() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numStructs();
}

lyric_object::TemplateWalker
lyric_object::LyricObject::getTemplate(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return TemplateWalker(m_reader, index);
}

int
lyric_object::LyricObject::numTemplates() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numTemplates();
}

lyric_object::TypeWalker
lyric_object::LyricObject::getType(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return TypeWalker(m_reader, index);
}

int
lyric_object::LyricObject::numTypes() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numTypes();
}

lyric_object::SymbolWalker
lyric_object::LyricObject::getSymbol(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return SymbolWalker(m_reader, index);
}

lyric_object::SymbolWalker
lyric_object::LyricObject::findSymbol(const lyric_common::SymbolPath &symbolPath) const
{
    if (m_reader == nullptr)
        return {};
    auto index = m_reader->getSymbolIndex(symbolPath);
    return SymbolWalker(m_reader, index);
}

int lyric_object::LyricObject::numSymbols() const
{
    if (!isValid())
        return 0;
    return m_reader->numSymbols();
}

lyric_common::SymbolPath
lyric_object::LyricObject::getSymbolPath(LinkageSection section, tu_uint32 index) const
{
    if (!isValid())
        return {};
    return m_reader->getSymbolPath(internal::linkage_to_descriptor_section(section), index);
}

bool
lyric_object::LyricObject::hasPlugin() const
{
    if (!isValid())
        return false;
    return m_reader->hasPlugin();
}

lyric_object::PluginWalker
lyric_object::LyricObject::getPlugin() const
{
    if (!isValid())
        return {};
    auto *pluginDescriptor = m_reader->getPlugin();
    if (pluginDescriptor == nullptr)
        return {};
    return PluginWalker(m_reader, (void *) pluginDescriptor);
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

std::string
lyric_object::LyricObject::dumpJson() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->dumpJson();
}

bool
lyric_object::LyricObject::verify(std::span<const tu_uint8> bytes)
{
    flatbuffers::Verifier verifier(bytes.data(), bytes.size());
    return lyo1::VerifyObjectBuffer(verifier);
}
