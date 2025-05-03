#ifndef LYRIC_RUNTIME_ABSTRACT_LOADER_H
#define LYRIC_RUNTIME_ABSTRACT_LOADER_H

#include <lyric_common/module_location.h>
#include <lyric_object/lyric_object.h>
#include <lyric_object/plugin_specifier.h>
#include <tempo_utils/option_template.h>
#include <tempo_utils/result.h>

#include "abstract_plugin.h"

namespace lyric_runtime {

    /**
     * AbstractLoader is an interface which defines how to load an object from a module location.
     */
    class AbstractLoader {

    public:
        virtual ~AbstractLoader() = default;

        /**
         * Returns a Result containing true if the loader can load an object at the specified location,
         * a Result containing false if the loader cannot load an object at the specified location, or a
         * Status if there was an error.
         *
         * @param location The location.
         * @return A Result containing a boolean indicating whether the loader can load the object
         */
        virtual tempo_utils::Result<bool> hasModule(
            const lyric_common::ModuleLocation &location) const = 0;

        /**
         * Loads the module at the specified location. A Result containing and Option containing the object is
         * returned if the load was successful. If the loader is not capable of loading the module then a Result
         * containing an empty Option is returned. If there was an error during loading then a Status is returned.
         *
         * @param location The location.
         * @return A Result containing an Option containing a valid LyricObject if loading was successful, or an
         * empty Option if the loader is not capable of loading from the location, or a Status.
         */
        virtual tempo_utils::Result<Option<lyric_object::LyricObject>> loadModule(
            const lyric_common::ModuleLocation &location) = 0;

        /**
         *
         * @param location
         * @param specifier
         * @return
         */
        virtual tempo_utils::Result<Option<std::shared_ptr<const AbstractPlugin>>> loadPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) = 0;
    };
}

#endif // LYRIC_RUNTIME_ABSTRACT_LOADER_H