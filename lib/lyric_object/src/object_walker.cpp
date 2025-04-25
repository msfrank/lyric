
#include <lyric_object/object_walker.h>
#include <lyric_object/internal/object_reader.h>

lyric_object::ObjectWalker::ObjectWalker()
{
}

lyric_object::ObjectWalker::ObjectWalker(std::shared_ptr<const internal::ObjectReader> reader)
    : m_reader(reader)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::ObjectWalker::ObjectWalker(const ObjectWalker &other)
    : m_reader(other.m_reader)
{
}

bool
lyric_object::ObjectWalker::isValid() const
{
    return m_reader && m_reader->isValid();
}

uint32_t
lyric_object::ObjectWalker::getVersionMajor() const
{
    if (m_reader == nullptr)
        return INVALID_ADDRESS_U32;
    return m_reader->getVersionMajor();
}

uint32_t
lyric_object::ObjectWalker::getVersionMinor() const
{
    if (m_reader == nullptr)
        return INVALID_ADDRESS_U32;
    return m_reader->getVersionMinor();
}

uint32_t
lyric_object::ObjectWalker::getVersionPatch() const
{
    if (m_reader == nullptr)
        return INVALID_ADDRESS_U32;
    return m_reader->getVersionPatch();
}

std::string
lyric_object::ObjectWalker::getVersionString() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->getVersionString();
}

lyric_object::ActionWalker
lyric_object::ObjectWalker::getAction(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return ActionWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numActions() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numActions();
}

lyric_object::BindingWalker
lyric_object::ObjectWalker::getBinding(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return BindingWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numBindings() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numBindings();
}

lyric_object::CallWalker
lyric_object::ObjectWalker::getCall(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return CallWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numCalls() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numCalls();
}

lyric_object::ClassWalker
lyric_object::ObjectWalker::getClass(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return ClassWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numClasses() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numClasses();
}

lyric_object::ConceptWalker
lyric_object::ObjectWalker::getConcept(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return ConceptWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numConcepts() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numConcepts();
}

lyric_object::EnumWalker
lyric_object::ObjectWalker::getEnum(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return EnumWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numEnums() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numEnums();
}

lyric_object::ExistentialWalker
lyric_object::ObjectWalker::getExistential(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return ExistentialWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numExistentials() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numExistentials();
}

lyric_object::FieldWalker
lyric_object::ObjectWalker::getField(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return FieldWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numFields() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numFields();
}

lyric_object::ImplWalker
lyric_object::ObjectWalker::getImpl(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return ImplWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numImpls() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numImpls();
}

lyric_object::ImportWalker
lyric_object::ObjectWalker::getImport(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return ImportWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numImports() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numImports();
}

lyric_object::InstanceWalker
lyric_object::ObjectWalker::getInstance(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return InstanceWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numInstances() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numInstances();
}

lyric_object::LinkWalker
lyric_object::ObjectWalker::getLink(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return LinkWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numLinks() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numLinks();
}

lyric_object::LiteralWalker
lyric_object::ObjectWalker::getLiteral(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return LiteralWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numLiterals() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numLiterals();
}

lyric_object::NamespaceWalker
lyric_object::ObjectWalker::getNamespace(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return NamespaceWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numNamespaces() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numNamespaces();
}

lyric_object::StaticWalker
lyric_object::ObjectWalker::getStatic(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return StaticWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numStatics() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numStatics();
}

lyric_object::StructWalker
lyric_object::ObjectWalker::getStruct(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return StructWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numStructs() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numStructs();
}

lyric_object::TemplateWalker
lyric_object::ObjectWalker::getTemplate(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return TemplateWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numTemplates() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numTemplates();
}

lyric_object::TypeWalker
lyric_object::ObjectWalker::getType(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return TypeWalker(m_reader, index);
}

int
lyric_object::ObjectWalker::numTypes() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->numTypes();
}

lyric_object::SymbolWalker
lyric_object::ObjectWalker::getSymbol(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return SymbolWalker(m_reader, index);
}

lyric_object::SymbolWalker
lyric_object::ObjectWalker::findSymbol(const lyric_common::SymbolPath &symbolPath) const
{
    if (m_reader == nullptr)
        return {};
    auto index = m_reader->getSymbolIndex(symbolPath);
    return SymbolWalker(m_reader, index);
}

int lyric_object::ObjectWalker::numSymbols() const
{
    if (!isValid())
        return 0;
    return m_reader->numSymbols();
}

lyric_common::SymbolPath
lyric_object::ObjectWalker::getSymbolPath(LinkageSection section, tu_uint32 index) const
{
    if (!isValid())
        return {};
    return m_reader->getSymbolPath(internal::linkage_to_descriptor_section(section), index);
}

bool
lyric_object::ObjectWalker::hasPlugin() const
{
    if (!isValid())
        return false;
    return m_reader->hasPlugin();
}

lyric_object::PluginWalker
lyric_object::ObjectWalker::getPlugin() const
{
    if (!isValid())
        return {};
    auto *pluginDescriptor = m_reader->getPlugin();
    if (pluginDescriptor == nullptr)
        return {};
    return PluginWalker(m_reader, (void *) pluginDescriptor);
}