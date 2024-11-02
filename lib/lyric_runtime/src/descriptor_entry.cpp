
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/descriptor_entry.h>

lyric_runtime::DescriptorEntry::DescriptorEntry(
    DescriptorTable *descriptorTable,
    tu_uint32 index)
    : m_descriptorTable(descriptorTable),
      m_index(index)
{
    TU_ASSERT (m_descriptorTable != nullptr);
    TU_ASSERT (m_index != lyric_object::INVALID_ADDRESS_U32);
}

lyric_runtime::BytecodeSegment *
lyric_runtime::DescriptorEntry::getSegment() const
{
    return m_descriptorTable->getSegment();
}

lyric_object::LinkageSection
lyric_runtime::DescriptorEntry::getLinkageSection() const
{
    return m_descriptorTable->getLinkageSection();
}

tu_uint32
lyric_runtime::DescriptorEntry::getDescriptorIndex() const
{
    return m_index;
}

lyric_common::SymbolUrl
lyric_runtime::DescriptorEntry::getSymbolUrl() const
{
    auto *segment = m_descriptorTable->getSegment();
    auto location = segment->getLocation();
    auto object = segment->getObject().getObject();
    auto symbolPath = object.getSymbolPath(m_descriptorTable->getLinkageSection(), m_index);
    return lyric_common::SymbolUrl(location, symbolPath);
}

tu_uint32
lyric_runtime::DescriptorEntry::getSegmentIndex() const
{
    return m_descriptorTable->getSegment()->getSegmentIndex();
}

lyric_runtime::DescriptorTable::DescriptorTable(BytecodeSegment *segment, lyric_object::LinkageSection section)
    : m_segment(segment),
      m_section(section),
      m_numDescriptors(lyric_runtime::INVALID_ADDRESS_U32),
      m_descriptorEntries(nullptr)
{
    TU_ASSERT (m_segment != nullptr);
    TU_ASSERT (m_section != lyric_object::LinkageSection::Invalid);
}

lyric_runtime::DescriptorTable::~DescriptorTable()
{
    if (m_numDescriptors != INVALID_ADDRESS_U32 && m_descriptorEntries != nullptr) {
        for (tu_uint32 i = 0; i < m_numDescriptors; i++) {
            auto *entry = m_descriptorEntries[i];
            delete entry;
        }
        std::free(m_descriptorEntries);
    }
}

lyric_runtime::BytecodeSegment *
lyric_runtime::DescriptorTable::getSegment() const
{
    return m_segment;
}

lyric_object::LinkageSection
lyric_runtime::DescriptorTable::getLinkageSection() const
{
    return m_section;
}

lyric_runtime::DescriptorEntry *
lyric_runtime::DescriptorTable::lookupDescriptor(tu_uint32 index)
{
    if (m_numDescriptors == INVALID_ADDRESS_U32) {
        auto object = m_segment->getObject().getObject();

        tu_uint32 numDescriptors;
        switch (m_section) {
            case lyric_object::LinkageSection::Action:
                numDescriptors = object.numActions();
                break;
            case lyric_object::LinkageSection::Call:
                numDescriptors = object.numCalls();
                break;
            case lyric_object::LinkageSection::Class:
                numDescriptors = object.numClasses();
                break;
            case lyric_object::LinkageSection::Concept:
                numDescriptors = object.numConcepts();
                break;
            case lyric_object::LinkageSection::Enum:
                numDescriptors = object.numEnums();
                break;
            case lyric_object::LinkageSection::Existential:
                numDescriptors = object.numExistentials();
                break;
            case lyric_object::LinkageSection::Field:
                numDescriptors = object.numFields();
                break;
            case lyric_object::LinkageSection::Instance:
                numDescriptors = object.numInstances();
                break;
            case lyric_object::LinkageSection::Namespace:
                numDescriptors = object.numNamespaces();
                break;
            case lyric_object::LinkageSection::Static:
                numDescriptors = object.numStatics();
                break;
            case lyric_object::LinkageSection::Struct:
                numDescriptors = object.numStructs();
                break;
            case lyric_object::LinkageSection::Type:
                numDescriptors = object.numTypes();
                break;
            default:
                return nullptr;
        }

        if (numDescriptors <= index)
            return nullptr;

        m_descriptorEntries = (DescriptorEntry **) std::calloc(m_numDescriptors, sizeof(DescriptorEntry *));
        m_numDescriptors = numDescriptors;
    }

    auto *currentEntry = m_descriptorEntries[index];
    if (currentEntry == nullptr) {
        currentEntry = new DescriptorEntry(this, index);
        m_descriptorEntries[index] = currentEntry;
    }

    return currentEntry;
}
