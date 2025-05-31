
#include <lyric_build/internal/build_macros.h>
#include <lyric_assembler/internal/allocator_trap_macro.h>
#include <lyric_assembler/internal/plugin_macro.h>
#include <lyric_assembler/internal/trap_macro.h>
#include <lyric_compiler/internal/push_result_macro.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/macro_registry.h>

tempo_utils::Result<std::shared_ptr<lyric_rewriter::MacroRegistry>>
lyric_build::internal::make_build_macros()
{
    auto macroRegistry = std::make_shared<lyric_rewriter::MacroRegistry>();

    macroRegistry->registerMacroName("AllocatorTrap", []() {
        return std::make_shared<lyric_assembler::internal::AllocatorTrapMacro>();
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
