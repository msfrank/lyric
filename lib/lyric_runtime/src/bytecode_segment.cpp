
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/internal/assembly_reader.h>
#include <tempo_utils/log_stream.h>


lyric_runtime::BytecodeSegment::BytecodeSegment(
    uint32_t segmentIndex,
    const lyric_common::AssemblyLocation &location,
    const lyric_object::LyricObject &object,
    std::shared_ptr<const AbstractPlugin> plugin)
    : m_segmentIndex(segmentIndex),
      m_location(location),
      m_object(object),
      m_plugin(plugin),
      m_data(nullptr)
{
    TU_ASSERT (m_object.isValid());
    m_bytecodeSize = m_object.getBytecodeSize();
    m_bytecode = m_object.getBytecodeData();

    auto reader = m_object.getObject();
    TU_ASSERT (reader.isValid());
    m_numLinks = reader.numLinks();
    m_links = m_numLinks > 0 ? new LinkEntry[m_numLinks] : nullptr;
    m_numStatics = reader.numStatics();
    m_statics = m_numStatics > 0 ? new DataCell[m_numStatics] : nullptr;
    m_numInstances = reader.numInstances();
    m_instances = m_numInstances > 0 ? new DataCell[m_numInstances] : nullptr;
    m_numEnums = reader.numEnums();
    m_enums = m_numEnums > 0 ? new DataCell[m_numEnums] : nullptr;
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
}

lyric_common::AssemblyLocation
lyric_runtime::BytecodeSegment::getLocation() const
{
    return m_location;
}

lyric_object::LyricObject
lyric_runtime::BytecodeSegment::getObject() const
{
    return m_object;
}

uint32_t
lyric_runtime::BytecodeSegment::getSegmentIndex() const
{
    return m_segmentIndex;
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

lyric_runtime::NativeFunc
lyric_runtime::BytecodeSegment::getTrap(tu_uint32 address) const
{
    if (m_plugin == nullptr)
        return nullptr;
    return m_plugin->getTrap(address);
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
