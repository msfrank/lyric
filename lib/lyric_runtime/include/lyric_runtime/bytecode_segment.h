#ifndef LYRIC_RUNTIME_BYTECODE_SEGMENT_H
#define LYRIC_RUNTIME_BYTECODE_SEGMENT_H

#include <lyric_object/lyric_object.h>

#include "data_cell.h"
#include "runtime_types.h"
#include "abstract_plugin.h"

namespace lyric_runtime {

    class BytecodeSegment {

    public:
        BytecodeSegment(
            tu_uint32 segmentIndex,
            const lyric_common::AssemblyLocation &location,
            const lyric_object::LyricObject &object,
            std::shared_ptr<const AbstractPlugin> plugin);
        ~BytecodeSegment();

        tu_uint32 getSegmentIndex() const;
        lyric_common::AssemblyLocation getLocation() const;
        lyric_object::LyricObject getObject() const;
        const tu_uint8 *getBytecodeData() const;
        tu_uint32 getBytecodeSize() const;

        const LinkEntry *getLink(tu_uint32 index) const;
        bool setLink(tu_uint32 index, const LinkEntry &entry);

        DataCell getStatic(tu_uint32 index) const;
        bool setStatic(tu_uint32 index, const DataCell &value);

        DataCell getInstance(tu_uint32 index) const;
        bool setInstance(tu_uint32 index, const DataCell &value);

        DataCell getEnum(tu_uint32 index) const;
        bool setEnum(tu_uint32 index, const DataCell &value);

        NativeFunc getTrap(tu_uint32 address) const;

    private:
        tu_uint32 m_segmentIndex;
        lyric_common::AssemblyLocation m_location;
        lyric_object::LyricObject m_object;
        std::shared_ptr<const AbstractPlugin> m_plugin;

        tu_uint32 m_bytecodeSize;
        tu_uint32 m_numLinks;
        tu_uint32 m_numStatics;
        tu_uint32 m_numInstances;
        tu_uint32 m_numEnums;

        const tu_uint8 *m_bytecode;
        LinkEntry *m_links;
        DataCell *m_statics;
        DataCell *m_instances;
        DataCell *m_enums;
    };
}

#endif // LYRIC_RUNTIME_BYTECODE_SEGMENT_H
