#ifndef LYRIC_RUNTIME_TYPE_ENTRY_H
#define LYRIC_RUNTIME_TYPE_ENTRY_H

#include <lyric_object/object_types.h>

namespace lyric_runtime {

    class BytecodeSegment;
    class TypeTable;

    class TypeEntry {

    public:
        TypeEntry(TypeTable *typeTable, tu_uint32 index);

        BytecodeSegment *getSegment() const;
        tu_uint32 getSegmentIndex() const;
        tu_uint32 getDescriptorIndex() const;

        lyric_common::TypeDef getTypeDef() const;

    private:
        TypeTable *m_typeTable;
        tu_uint32 m_index;
    };

    class TypeTable {
    public:
        explicit TypeTable(BytecodeSegment *segment);
        ~TypeTable();

        BytecodeSegment *getSegment() const;

        TypeEntry *lookupType(tu_uint32 index);

    private:
        BytecodeSegment *m_segment;
        tu_uint32 m_numTypes;
        TypeEntry **m_typeEntries;
    };
}

#endif // LYRIC_RUNTIME_TYPE_ENTRY_H
