#ifndef LYRIC_OBJECT_PLUGIN_WALKER_H
#define LYRIC_OBJECT_PLUGIN_WALKER_H

#include <lyric_common/module_location.h>

#include "object_types.h"

namespace lyric_object {

    class PluginWalker {

    public:
        PluginWalker();
        PluginWalker(const PluginWalker &other);

        bool isValid() const;

        lyric_common::ModuleLocation getPluginLocation() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_pluginDescriptor = nullptr;

        PluginWalker(std::shared_ptr<const internal::ObjectReader> reader, void *pluginDescriptor);

        friend class LyricObject;
    };
}

#endif // LYRIC_OBJECT_PLUGIN_WALKER_H
