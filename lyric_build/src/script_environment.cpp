//
//#include <lyric_build/script_environment.h>
//#include <lyric_runtime/generated/assembly.h>
//#include <lyric_runtime/internal/assembly_reader.h>
//
//lyric_build::ScriptEnvironment::ScriptEnvironment()
//{
//}
//
//lyric_build::ScriptEnvironment::ScriptEnvironment(
//    const absl::flat_hash_map<lyric_common::AssemblyLocation,lyric_runtime::LyricAssembly> &assemblies)
//    : m_assemblies(assemblies)
//{
//}
//
//lyric_build::ScriptEnvironment::ScriptEnvironment(const ScriptEnvironment &other)
//    : m_assemblies(other.m_assemblies)
//{
//}
//
//absl::flat_hash_set<lyric_common::SymbolUrl>
//lyric_build::ScriptEnvironment::getSymbols() const
//{
//    return m_symbols;
//}
//
//tempo_utils::Result<bool>
//lyric_build::ScriptEnvironment::hasAssembly(const lyric_common::AssemblyLocation &location) const
//{
//    return m_assemblies.contains(location);
//}
//
//tempo_utils::Result<Option<lyric_common::AssemblyLocation>>
//lyric_build::ScriptEnvironment::resolveAssembly(const lyric_common::AssemblyLocation &location) const
//{
//    return Option<lyric_common::AssemblyLocation>();
//}
//
//tempo_utils::Result<Option<lyric_runtime::LyricAssembly>>
//lyric_build::ScriptEnvironment::loadAssembly(const lyric_common::AssemblyLocation &location)
//{
//    if (m_assemblies.contains(location))
//        return Option(m_assemblies[location]);
//    return Option<lyric_runtime::LyricAssembly>();
//}
//
//tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
//lyric_build::ScriptEnvironment::loadPlugin(
//    const lyric_common::AssemblyLocation &location,
//    const lyric_runtime::PluginSpecifier &specifier)
//{
//    return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>();
//}
//
//void
//lyric_build::ScriptEnvironment::storeAssembly(
//    const lyric_common::AssemblyLocation &location,
//    const lyric_runtime::LyricAssembly &assembly)
//{
//    if (!assembly.isValid())
//        return;
//    m_assemblies[location] = assembly;
//
//    auto reader = assembly.getReader();
//    for (uint32_t i = 0; i < reader->numSymbols(); i++) {
//        auto *symbol = reader->getSymbol(i);
//        auto path = lyric_common::SymbolPath::fromString(symbol->fqsn()->str());
//        if (path == lyric_common::SymbolPath())
//            continue;
//        if (path.getPath().size() > 1)
//            continue;
//        lyric_common::SymbolUrl symbolUrl(location, path);
//        m_symbols.insert(symbolUrl);
//    }
//}