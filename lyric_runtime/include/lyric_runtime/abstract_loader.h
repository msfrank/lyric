#ifndef LYRIC_RUNTIME_ABSTRACT_LOADER_H
#define LYRIC_RUNTIME_ABSTRACT_LOADER_H

#include <lyric_common/assembly_location.h>
#include <lyric_object/lyric_object.h>
#include <lyric_object/plugin_specifier.h>
#include <tempo_utils/option_template.h>
#include <tempo_utils/result.h>

#include "abstract_plugin.h"

namespace lyric_runtime {

    /**
     * AbstractLoader is an interface which defines how to load an assembly from an assembly location.
     */
    class AbstractLoader {

    public:
        virtual ~AbstractLoader() = default;

        /**
         * Returns a Result containing true if the loader can load an assembly at the specified location,
         * a Result containing false if the loader cannot load an assembly at the specified location, or a
         * Status if there was an error.
         *
         * @param location The location.
         * @return A Result containing a boolean indicating whether the loader can load the assembly
         */
        virtual tempo_utils::Result<bool> hasAssembly(
            const lyric_common::AssemblyLocation &location) const = 0;

        /**
         * Returns a Result containing a fully qualified location for the specified location, a Result containing
         * an invalid location if the loader cannot fully qualify the location, or a Status if there was an error.
         *
         * @param location The location.
         * @return A Result containing the fully qualified location if resolution was successful, or an invalid
         *     location if the loader is not capable of resolving the location.
         */
        virtual tempo_utils::Result<Option<lyric_common::AssemblyLocation>> resolveAssembly(
            const lyric_common::AssemblyLocation &location) const = 0;

        /**
         * Loads the assembly at the specified location. A Result containing the assembly is returned if the load
         * was successful. If the loader is not capable of loading the assembly then a Result containing an invalid
         * assembly is returned. If there was an error during loading then a Status is returned.
         *
         * @param location The location.
         * @return A Result containing a valid LyricAssembly if loading was successful, or an invalid assembly if
         *     the loader is not capable of loading from the location.
         */
        virtual tempo_utils::Result<Option<lyric_object::LyricObject>> loadAssembly(
            const lyric_common::AssemblyLocation &location) = 0;

        /**
         *
         * @param location
         * @param specifier
         * @return
         */
        virtual tempo_utils::Result<Option<std::shared_ptr<const AbstractPlugin>>> loadPlugin(
            const lyric_common::AssemblyLocation &location,
            const lyric_object::PluginSpecifier &specifier) = 0;
    };
}

#endif // LYRIC_RUNTIME_ABSTRACT_LOADER_H