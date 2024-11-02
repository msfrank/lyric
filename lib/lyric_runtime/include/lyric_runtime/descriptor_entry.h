#ifndef LYRIC_RUNTIME_DESCRIPTOR_ENTRY_H
#define LYRIC_RUNTIME_DESCRIPTOR_ENTRY_H

#include <lyric_object/object_types.h>

namespace lyric_runtime {

    class BytecodeSegment;
    class DescriptorTable;

    class DescriptorEntry {

    public:
        DescriptorEntry(DescriptorTable *descriptorTable, tu_uint32 index);

        BytecodeSegment *getSegment() const;
        lyric_object::LinkageSection getLinkageSection() const;
        tu_uint32 getDescriptorIndex() const;

        lyric_common::SymbolUrl getSymbolUrl() const;
        tu_uint32 getSegmentIndex() const;

    private:
        DescriptorTable *m_descriptorTable;
        tu_uint32 m_index;
    };

    class DescriptorTable {
    public:
        DescriptorTable(BytecodeSegment *segment, lyric_object::LinkageSection section);
        ~DescriptorTable();

        BytecodeSegment *getSegment() const;
        lyric_object::LinkageSection getLinkageSection() const;

        DescriptorEntry *lookupDescriptor(tu_uint32 index);

    private:
        BytecodeSegment *m_segment;
        lyric_object::LinkageSection m_section;
        tu_uint32 m_numDescriptors;
        DescriptorEntry **m_descriptorEntries;
    };
}

#endif // LYRIC_RUNTIME_DESCRIPTOR_ENTRY_H
