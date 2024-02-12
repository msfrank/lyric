//#ifndef LYRIC_BUILD_COMPILE_SCRIPT_TASK_H
//#define LYRIC_BUILD_COMPILE_SCRIPT_TASK_H
//
//#include <absl/container/flat_hash_set.h>
//#include <boost/uuid/uuid.hpp>
//
//#include <lyric_compiler/lyric_compiler.h>
//#include <lyric_parser/lyric_parser.h>
//
//#include "base_task.h"
//
//class CompileScriptTask : public BaseTask {
//
//    enum class CompileScriptPhase {
//        SYMBOLIZE_SCRIPT,
//        COMPILE_SCRIPT,
//        COMPLETE,
//    };
//
//public:
//    CompileScriptTask(const boost::uuids::uuid &generation, const TaskKey &key);
//
//    lyric_build::BuildResult<std::string> configureTask(const ConfigStore *config) override;
//    lyric_build::BuildResult<absl::flat_hash_set<TaskKey>> checkDependencies() override;
//    lyric_build::BuildStatus runTask(const absl::flat_hash_map<TaskKey,TaskState> &deps, BuildState *generation) override;
//
//private:
//    std::string m_scriptData;
//    absl::flat_hash_set<lyric_runtime::SymbolUrl> m_envSymbols;
//    lyric_parser::ParserOptions m_parserOptions;
//    CompilerOptions m_compilerOptions;
//    AssemblyLocation m_scriptLocation;
//    absl::flat_hash_set<TaskKey> m_compileTargets;
//    CompileScriptPhase m_phase;
//    LyricArchetype m_archetype;
//
//    lyric_build::BuildStatus configure(const ConfigStore *config);
//    lyric_build::BuildStatus symbolizeScript(const absl::flat_hash_map<TaskKey,TaskState> &deps, BuildState *buildState);
//    lyric_build::BuildStatus compileScript(const absl::flat_hash_map<TaskKey,TaskState> &deps, BuildState *buildState);
//};
//
//BaseTask *new_compile_script_task(const boost::uuids::uuid &generation, const TaskKey &key);
//
//#endif // LYRIC_BUILD_COMPILE_SCRIPT_TASK_H