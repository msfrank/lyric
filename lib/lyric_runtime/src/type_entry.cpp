
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/type_entry.h>

lyric_runtime::TypeEntry::TypeEntry(
    TypeTable *typeTable,
    tu_uint32 index)
    : m_typeTable(typeTable),
      m_index(index)
{
    TU_ASSERT (m_typeTable != nullptr);
    TU_ASSERT (m_index != lyric_object::INVALID_ADDRESS_U32);
}

lyric_runtime::BytecodeSegment *
lyric_runtime::TypeEntry::getSegment() const
{
    return m_typeTable->getSegment();
}

tu_uint32
lyric_runtime::TypeEntry::getSegmentIndex() const
{
    return m_typeTable->getSegment()->getSegmentIndex();
}

tu_uint32
lyric_runtime::TypeEntry::getDescriptorIndex() const
{
    return m_index;
}

lyric_common::TypeDef
lyric_runtime::TypeEntry::getTypeDef() const
{
    auto *segment = m_typeTable->getSegment();
    auto object = segment->getObject().getObject();
    auto type = object.getType(m_index);
    return type.getTypeDef();
}

lyric_runtime::TypeTable::TypeTable(BytecodeSegment *segment)
    : m_segment(segment),
      m_numTypes(lyric_runtime::INVALID_ADDRESS_U32),
      m_typeEntries(nullptr)
{
    TU_ASSERT (m_segment != nullptr);
}

lyric_runtime::TypeTable::~TypeTable()
{
    if (m_numTypes != INVALID_ADDRESS_U32 && m_typeEntries != nullptr) {
        for (tu_uint32 i = 0; i < m_numTypes; i++) {
            auto *entry = m_typeEntries[i];
            delete entry;
        }
        std::free(m_typeEntries);
    }
}

lyric_runtime::BytecodeSegment *
lyric_runtime::TypeTable::getSegment() const
{
    return m_segment;
}

lyric_runtime::TypeEntry *
lyric_runtime::TypeTable::lookupType(tu_uint32 index)
{
    if (m_numTypes == INVALID_ADDRESS_U32) {
        auto object = m_segment->getObject().getObject();
        tu_uint32 numTypes = object.numTypes();

        if (numTypes <= index)
            return nullptr;

        m_typeEntries = (TypeEntry **) std::calloc(m_numTypes, sizeof(TypeEntry *));
        m_numTypes = numTypes;
    }

    auto *currentEntry = m_typeEntries[index];
    if (currentEntry == nullptr) {
        currentEntry = new TypeEntry(this, index);
        m_typeEntries[index] = currentEntry;
    }

    return currentEntry;
}
