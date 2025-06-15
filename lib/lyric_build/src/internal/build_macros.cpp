
#include <lyric_build/internal/build_macros.h>
#include <lyric_assembler/internal/allocator_trap_macro.h>
#include <lyric_assembler/internal/load_data_macro.h>
#include <lyric_assembler/internal/plugin_macro.h>
#include <lyric_assembler/internal/store_data_macro.h>
#include <lyric_assembler/internal/trap_macro.h>
#include <lyric_compiler/internal/push_result_macro.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/macro_registry.h>

#include "lyric_assembler/assembler_macro_visitor.h"
#include "lyric_compiler/compiler_macro_visitor.h"
#include "lyric_schema/assembler_schema.h"
#include "lyric_schema/compiler_schema.h"

tempo_utils::Result<std::shared_ptr<lyric_rewriter::MacroRegistry>>
lyric_build::internal::make_build_macros()
{
    auto macroRegistry = std::make_shared<lyric_rewriter::MacroRegistry>();

    macroRegistry->registerMacroName("AllocatorTrap", []() {
        return std::make_shared<lyric_assembler::internal::AllocatorTrapMacro>();
    });
    macroRegistry->registerMacroName("LoadData", []() {
        return std::make_shared<lyric_assembler::internal::LoadDataMacro>();
    });
    macroRegistry->registerMacroName("StoreData", []() {
        return std::make_shared<lyric_assembler::internal::StoreDataMacro>();
    });
    macroRegistry->registerMacroName("Plugin", []() {
        return std::make_shared<lyric_assembler::internal::PluginMacro>();
    });
    macroRegistry->registerMacroName("PushResult", []() {
        return std::make_shared<lyric_compiler::internal::PushResultMacro>();
    });
    macroRegistry->registerMacroName("Trap", []() {
        return std::make_shared<lyric_assembler::internal::TrapMacro>();
    });

    return macroRegistry;
}

tempo_utils::Result<std::shared_ptr<lyric_rewriter::VisitorRegistry>>
lyric_build::internal::make_build_visitors()
{
    auto visitorRegistry = std::make_shared<lyric_rewriter::VisitorRegistry>();

    visitorRegistry->registerVisitorNamespace(
        tempo_utils::Url::fromString(lyric_schema::kLyricAssemblerNs.getNs()),
        lyric_assembler::make_assembler_visitor);
    visitorRegistry->registerVisitorNamespace(
        tempo_utils::Url::fromString(lyric_schema::kLyricCompilerNs.getNs()),
        lyric_compiler::make_compiler_visitor);

    return visitorRegistry;
}
