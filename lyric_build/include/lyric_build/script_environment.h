//#ifndef LYRIC_BUILD_SCRIPT_ENVIRONMENT_H
//#define LYRIC_BUILD_SCRIPT_ENVIRONMENT_H
//
//#include <absl/container/flat_hash_map.h>
//#include <absl/container/flat_hash_set.h>
//
//#include <lyric_common/symbol_url.h>
//#include <lyric_runtime/abstract_loader.h>
//
//namespace lyric_build {
//
//    class ScriptEnvironment : public lyric_runtime::AbstractLoader {
//
//    public:
//        ScriptEnvironment();
//        ScriptEnvironment(
//            const absl::flat_hash_map<lyric_common::AssemblyLocation, lyric_runtime::LyricAssembly> &assemblies);
//        ScriptEnvironment(const ScriptEnvironment &other);
//
//        absl::flat_hash_set<lyric_common::SymbolUrl> getSymbols() const;
//
//        tempo_utils::Result<bool> hasAssembly(const lyric_common::AssemblyLocation &location) const override;
//        tempo_utils::Result<Option < lyric_common::AssemblyLocation>> resolveAssembly(
//            const lyric_common::AssemblyLocation &location) const override;
//        tempo_utils::Result<Option < lyric_runtime::LyricAssembly>> loadAssembly(
//            const lyric_common::AssemblyLocation &location) override;
//        tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>> loadPlugin(
//            const lyric_common::AssemblyLocation &location,
//            const lyric_runtime::PluginSpecifier &specifier) override;
//
//        void storeAssembly(
//            const lyric_common::AssemblyLocation &location,
//            const lyric_runtime::LyricAssembly &assembly);
//
//    private:
//        absl::flat_hash_map<lyric_common::AssemblyLocation, lyric_runtime::LyricAssembly> m_assemblies;
//        absl::flat_hash_set<lyric_common::SymbolUrl> m_symbols;
//    };
//}
//
//#endif // LYRIC_BUILD_SCRIPT_ENVIRONMENT_H
