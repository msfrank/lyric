
#include <lyric_runtime/interpreter_result.h>
#include <lyric_runtime/static_loader.h>

tempo_utils::Result<bool>
lyric_runtime::StaticLoader::hasModule(const lyric_common::ModuleLocation &location) const
{
    return m_objects.contains(location);
}

tempo_utils::Result<Option<lyric_object::LyricObject>>
lyric_runtime::StaticLoader::loadModule(const lyric_common::ModuleLocation &location)
{
    auto entry = m_objects.find(location);
    if (entry != m_objects.cend())
        return Option(entry->second);
    return Option<lyric_object::LyricObject>();
}

tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
lyric_runtime::StaticLoader::loadPlugin(
    const lyric_common::ModuleLocation &location,
    const lyric_object::PluginSpecifier &specifier)
{
    return Option<std::shared_ptr<const AbstractPlugin>>();
}

tempo_utils::Status
lyric_runtime::StaticLoader::insertModule(
    const lyric_common::ModuleLocation &location,
    const lyric_object::LyricObject &object)
{
    if (m_objects.contains(location))
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "static loader already contains object {}", location.toString());
    m_objects[location] = object;
    return {};
}