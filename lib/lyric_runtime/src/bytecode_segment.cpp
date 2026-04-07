
#include <lyric_runtime/bytecode_segment.h>
#include <tempo_utils/log_stream.h>

lyric_runtime::BytecodeSegment::BytecodeSegment(
    uint32_t segmentIndex,
    bool isSystem,
    const lyric_common::ModuleLocation &objectLocation,
    const lyric_object::LyricObject &object,
    const lyric_common::ModuleLocation &pluginLocation,
    std::shared_ptr<const AbstractPlugin> plugin)
    : m_segmentIndex(segmentIndex),
      m_isSystem(isSystem),
      m_objectLocation(objectLocation),
      m_object(object),
      m_pluginLocation(pluginLocation),
      m_plugin(plugin),
      m_data(nullptr),
      m_actionDescriptors(this, lyric_object::LinkageSection::Action),
      m_callDescriptors(this, lyric_object::LinkageSection::Call),
      m_classDescriptors(this, lyric_object::LinkageSection::Class),
      m_conceptDescriptors(this, lyric_object::LinkageSection::Concept),
      m_enumDescriptors(this, lyric_object::LinkageSection::Enum),
      m_existentialDescriptors(this, lyric_object::LinkageSection::Existential),
      m_fieldDescriptors(this, lyric_object::LinkageSection::Field),
      m_instanceDescriptors(this, lyric_object::LinkageSection::Instance),
      m_namespaceDescriptors(this, lyric_object::LinkageSection::Namespace),
      m_protocolDescriptors(this, lyric_object::LinkageSection::Protocol),
      m_staticDescriptors(this, lyric_object::LinkageSection::Static),
      m_structDescriptors(this, lyric_object::LinkageSection::Struct),
      m_types(this)
{
    TU_ASSERT (m_objectLocation.isValid());
    TU_ASSERT (m_object.isValid());
    m_bytecodeSize = m_object.getBytecodeSize();
    m_bytecode = m_object.getBytecodeData();

    m_numLinks = m_object.numLinks();
    m_links = m_numLinks > 0 ? new LinkEntry[m_numLinks] : nullptr;
    m_numStatics = m_object.numStatics();
    m_statics = m_numStatics > 0 ? new DataCell[m_numStatics] : nullptr;
    m_numInstances = m_object.numInstances();
    m_instances = m_numInstances > 0 ? new DataCell[m_numInstances] : nullptr;
    m_numEnums = m_object.numEnums();
    m_enums = m_numEnums > 0 ? new DataCell[m_numEnums] : nullptr;
    m_numProtocols = m_object.numProtocols();
    m_protocols = m_numProtocols > 0 ? new DataCell[m_numProtocols] : nullptr;
    m_numNamespaces = m_object.numNamespaces();
    m_namespaces = m_numNamespaces > 0 ? new DataCell[m_numNamespaces] : nullptr;

    if (m_object.hasPlugin()) {
        TU_NOTNULL (m_plugin);
        auto walker = m_object.getPlugin();
        TU_ASSERT (walker.isValid());

        m_numTraps = walker.numTraps();
        m_traps = m_numTraps > 0 ? new const NativeTrap*[m_numTraps] : nullptr;

        absl::flat_hash_map<std::string,const NativeTrap *> nativeTraps;
        for (tu_uint32 i = 0; i < m_plugin->numTraps(); ++i) {
            auto *trap = m_plugin->getTrap(i);
            if (trap != nullptr && trap->name != nullptr && trap->func != nullptr) {
                std::string name(trap->name);
                nativeTraps[name] = trap;
            }
        }

        for (tu_uint32 i = 0; i < m_numTraps; ++i) {
            auto trapName = walker.getTrap(i);
            auto entry = nativeTraps.find(trapName);
            if (entry != nativeTraps.cend()) {
                m_traps[i] = entry->second;
            }
        }
    }
}

lyric_runtime::BytecodeSegment::~BytecodeSegment()
{
    // unload must happen first in the destructor
    if (m_plugin != nullptr) {
        m_plugin->unload(this);
    }

    // free the static storage area
    delete[] m_links;
    delete[] m_statics;
    delete[] m_instances;
    delete[] m_enums;
    delete[] m_protocols;
    delete[] m_namespaces;
    delete[] m_traps;
}

uint32_t
lyric_runtime::BytecodeSegment::getSegmentIndex() const
{
    return m_segmentIndex;
}

bool
lyric_runtime::BytecodeSegment::isSystem() const
{
    return m_isSystem;
}

lyric_common::ModuleLocation
lyric_runtime::BytecodeSegment::getObjectLocation() const
{
    return m_objectLocation;
}

lyric_object::LyricObject
lyric_runtime::BytecodeSegment::getObject() const
{
    return m_object;
}

lyric_common::ModuleLocation
lyric_runtime::BytecodeSegment::getPluginLocation() const
{
    return m_pluginLocation;
}

std::shared_ptr<const lyric_runtime::AbstractPlugin>
lyric_runtime::BytecodeSegment::getPlugin() const
{
    return m_plugin;
}

const uint8_t *
lyric_runtime::BytecodeSegment::getBytecodeData() const
{
    return m_bytecode;
}

uint32_t
lyric_runtime::BytecodeSegment::getBytecodeSize() const
{
    return m_bytecodeSize;
}

std::span<const tu_uint8>
lyric_runtime::BytecodeSegment::getBytecode() const
{
    return std::span(m_bytecode, m_bytecodeSize);
}

lyric_runtime::DescriptorEntry *
lyric_runtime::BytecodeSegment::lookupDescriptor(lyric_object::LinkageSection section, tu_uint32 index)
{
    switch (section) {
        case lyric_object::LinkageSection::Action:
            return m_actionDescriptors.lookupDescriptor(index);
        case lyric_object::LinkageSection::Call:
            return m_callDescriptors.lookupDescriptor(index);
        case lyric_object::LinkageSection::Class:
            return m_classDescriptors.lookupDescriptor(index);
        case lyric_object::LinkageSection::Concept:
            return m_conceptDescriptors.lookupDescriptor(index);
        case lyric_object::LinkageSection::Enum:
            return m_enumDescriptors.lookupDescriptor(index);
        case lyric_object::LinkageSection::Existential:
            return m_existentialDescriptors.lookupDescriptor(index);
        case lyric_object::LinkageSection::Field:
            return m_fieldDescriptors.lookupDescriptor(index);
        case lyric_object::LinkageSection::Instance:
            return m_instanceDescriptors.lookupDescriptor(index);
        case lyric_object::LinkageSection::Namespace:
            return m_namespaceDescriptors.lookupDescriptor(index);
        case lyric_object::LinkageSection::Protocol:
            return m_protocolDescriptors.lookupDescriptor(index);
        case lyric_object::LinkageSection::Static:
            return m_staticDescriptors.lookupDescriptor(index);
        case lyric_object::LinkageSection::Struct:
            return m_structDescriptors.lookupDescriptor(index);
        default:
            return nullptr;
    }
}

lyric_runtime::TypeEntry *
lyric_runtime::BytecodeSegment::lookupType(tu_uint32 index)
{
    return m_types.lookupType(index);
}

const lyric_runtime::LinkEntry *
lyric_runtime::BytecodeSegment::getLink(uint32_t index) const
{
    if (m_numLinks <= index)
        return nullptr;
    return &m_links[index];
}

bool
lyric_runtime::BytecodeSegment::setLink(uint32_t index, const LinkEntry &entry)
{
    if (m_numLinks <= index)
        return false;
    m_links[index] = entry;
    return true;
}

lyric_runtime::DataCell
lyric_runtime::BytecodeSegment::getStatic(uint32_t index) const
{
    if (m_numStatics <= index)
        return {};
    return m_statics[index];
}

bool
lyric_runtime::BytecodeSegment::setStatic(uint32_t index, const DataCell &value)
{
    if (m_numStatics <= index)
        return false;
    m_statics[index] = value;
    return true;
}

lyric_runtime::DataCell
lyric_runtime::BytecodeSegment::getInstance(uint32_t index) const
{
    if (m_numInstances <= index)
        return {};
    return m_instances[index];
}

bool
lyric_runtime::BytecodeSegment::setInstance(uint32_t index, const DataCell &value)
{
    if (m_numInstances <= index)
        return false;
    m_instances[index] = value;
    return true;
}

lyric_runtime::DataCell
lyric_runtime::BytecodeSegment::getEnum(uint32_t index) const
{
    if (m_numEnums <= index)
        return {};
    return m_enums[index];
}

bool
lyric_runtime::BytecodeSegment::setEnum(uint32_t index, const DataCell &value)
{
    if (m_numEnums <= index)
        return false;
    m_enums[index] = value;
    return true;
}

lyric_runtime::DataCell
lyric_runtime::BytecodeSegment::getProtocol(uint32_t index) const
{
    if (m_numProtocols <= index)
        return {};
    return m_protocols[index];
}

bool
lyric_runtime::BytecodeSegment::setProtocol(uint32_t index, const DataCell &value)
{
    if (m_numProtocols <= index)
        return false;
    m_protocols[index] = value;
    return true;
}

lyric_runtime::DataCell
lyric_runtime::BytecodeSegment::getNamespace(uint32_t index) const
{
    if (m_numNamespaces <= index)
        return {};
    return m_namespaces[index];
}

bool
lyric_runtime::BytecodeSegment::setNamespace(uint32_t index, const DataCell &value)
{
    if (m_numNamespaces <= index)
        return false;
    m_namespaces[index] = value;
    return true;
}

const lyric_runtime::NativeTrap *
lyric_runtime::BytecodeSegment::getTrap(tu_uint32 address) const
{
    if (address < m_numTraps)
        return m_traps[address];
    return nullptr;
}

void *
lyric_runtime::BytecodeSegment::getData() const
{
    return m_data;
}

void
lyric_runtime::BytecodeSegment::setData(void *data)
{
    m_data = data;
}
