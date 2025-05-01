#ifndef LYRIC_RUNTIME_BYTECODE_SEGMENT_H
#define LYRIC_RUNTIME_BYTECODE_SEGMENT_H

#include <lyric_object/lyric_object.h>

#include "abstract_plugin.h"
#include "data_cell.h"
#include "descriptor_entry.h"
#include "runtime_types.h"
#include "type_entry.h"

namespace lyric_runtime {

    /**
     * The bytecode segment is a runtime structure containing the readonly bytecode, virtual tables, and
     * static storage areas for the object at a specified module location.
     */
    class BytecodeSegment {

    public:
        BytecodeSegment(
            tu_uint32 segmentIndex,
            const lyric_common::ModuleLocation &objectLocation,
            const lyric_object::LyricObject &object,
            const lyric_common::ModuleLocation &pluginLocation,
            std::shared_ptr<const AbstractPlugin> plugin);
        ~BytecodeSegment();

        tu_uint32 getSegmentIndex() const;
        lyric_common::ModuleLocation getObjectLocation() const;
        lyric_object::LyricObject getObject() const;
        lyric_common::ModuleLocation getPluginLocation() const;
        std::shared_ptr<const AbstractPlugin> getPlugin() const;

        const tu_uint8 *getBytecodeData() const;
        tu_uint32 getBytecodeSize() const;

        DescriptorEntry *lookupDescriptor(lyric_object::LinkageSection section, tu_uint32 index);
        TypeEntry *lookupType(tu_uint32 index);

        const LinkEntry *getLink(tu_uint32 index) const;
        bool setLink(tu_uint32 index, const LinkEntry &entry);

        DataCell getStatic(tu_uint32 index) const;
        bool setStatic(tu_uint32 index, const DataCell &value);

        DataCell getInstance(tu_uint32 index) const;
        bool setInstance(tu_uint32 index, const DataCell &value);

        DataCell getEnum(tu_uint32 index) const;
        bool setEnum(tu_uint32 index, const DataCell &value);

        NativeFunc getTrap(tu_uint32 address) const;

        void *getData() const;
        void setData(void *data);

    private:
        tu_uint32 m_segmentIndex;
        lyric_common::ModuleLocation m_objectLocation;
        lyric_object::LyricObject m_object;
        lyric_common::ModuleLocation m_pluginLocation;
        std::shared_ptr<const AbstractPlugin> m_plugin;
        void *m_data;

        const tu_uint8 *m_bytecode;
        tu_uint32 m_bytecodeSize;

        LinkEntry *m_links;
        tu_uint32 m_numLinks;

        DataCell *m_statics;
        tu_uint32 m_numStatics;

        DataCell *m_instances;
        tu_uint32 m_numInstances;

        DataCell *m_enums;
        tu_uint32 m_numEnums;

        DescriptorTable m_actionDescriptors;
        DescriptorTable m_callDescriptors;
        DescriptorTable m_classDescriptors;
        DescriptorTable m_conceptDescriptors;
        DescriptorTable m_enumDescriptors;
        DescriptorTable m_existentialDescriptors;
        DescriptorTable m_fieldDescriptors;
        DescriptorTable m_instanceDescriptors;
        DescriptorTable m_namespaceDescriptors;
        DescriptorTable m_staticDescriptors;
        DescriptorTable m_structDescriptors;
        TypeTable m_types;
    };
}

#endif // LYRIC_RUNTIME_BYTECODE_SEGMENT_H
