
#include <lyric_build/internal/build_macros.h>
#include <lyric_assembler/assembler_macro_visitor.h>
#include <lyric_assembler/internal/allocator_trap_macro.h>
#include <lyric_assembler/internal/load_data_macro.h>
#include <lyric_assembler/internal/opcode_macro.h>
#include <lyric_assembler/internal/plugin_macro.h>
#include <lyric_assembler/internal/store_data_macro.h>
#include <lyric_assembler/internal/trap_macro.h>
#include <lyric_compiler/compiler_macro_visitor.h>
#include <lyric_compiler/internal/pop_result_macro.h>
#include <lyric_compiler/internal/push_result_macro.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/macro_registry.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_schema/compiler_schema.h>

tempo_utils::Result<std::shared_ptr<lyric_rewriter::MacroRegistry>>
lyric_build::internal::make_build_macros()
{
    auto macroRegistry = std::make_shared<lyric_rewriter::MacroRegistry>();

    // assembler macros

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
    macroRegistry->registerMacroName("Trap", []() {
        return std::make_shared<lyric_assembler::internal::TrapMacro>();
    });

    // assembler opcode macros

    macroRegistry->registerMacroName("Noop", []() {
        return std::make_shared<lyric_assembler::internal::NoopMacro>();
    });
    macroRegistry->registerMacroName("Pop", []() {
        return std::make_shared<lyric_assembler::internal::PopMacro>();
    });
    macroRegistry->registerMacroName("Dup", []() {
        return std::make_shared<lyric_assembler::internal::DupMacro>();
    });
    macroRegistry->registerMacroName("Pick", []() {
        return std::make_shared<lyric_assembler::internal::PickMacro>();
    });
    macroRegistry->registerMacroName("Drop", []() {
        return std::make_shared<lyric_assembler::internal::DropMacro>();
    });
    macroRegistry->registerMacroName("RPick", []() {
        return std::make_shared<lyric_assembler::internal::RPickMacro>();
    });
    macroRegistry->registerMacroName("I64Add", []() {
        return std::make_shared<lyric_assembler::internal::I64AddMacro>();
    });
    macroRegistry->registerMacroName("I64Sub", []() {
        return std::make_shared<lyric_assembler::internal::I64SubMacro>();
    });
    macroRegistry->registerMacroName("I64Mul", []() {
        return std::make_shared<lyric_assembler::internal::I64MulMacro>();
    });
    macroRegistry->registerMacroName("I64Div", []() {
        return std::make_shared<lyric_assembler::internal::I64DivMacro>();
    });
    macroRegistry->registerMacroName("I64Neg", []() {
        return std::make_shared<lyric_assembler::internal::I64NegMacro>();
    });
    macroRegistry->registerMacroName("DblAdd", []() {
        return std::make_shared<lyric_assembler::internal::DblAddMacro>();
    });
    macroRegistry->registerMacroName("DblSub", []() {
        return std::make_shared<lyric_assembler::internal::DblSubMacro>();
    });
    macroRegistry->registerMacroName("DblMul", []() {
        return std::make_shared<lyric_assembler::internal::DblMulMacro>();
    });
    macroRegistry->registerMacroName("DblDiv", []() {
        return std::make_shared<lyric_assembler::internal::DblDivMacro>();
    });
    macroRegistry->registerMacroName("DblNeg", []() {
        return std::make_shared<lyric_assembler::internal::DblNegMacro>();
    });
    macroRegistry->registerMacroName("BoolCmp", []() {
        return std::make_shared<lyric_assembler::internal::BoolCmpMacro>();
    });
    macroRegistry->registerMacroName("ChrCmp", []() {
        return std::make_shared<lyric_assembler::internal::ChrCmpMacro>();
    });
    macroRegistry->registerMacroName("I64Cmp", []() {
        return std::make_shared<lyric_assembler::internal::I64CmpMacro>();
    });
    macroRegistry->registerMacroName("DblCmp", []() {
        return std::make_shared<lyric_assembler::internal::DblCmpMacro>();
    });
    macroRegistry->registerMacroName("TypeCmp", []() {
        return std::make_shared<lyric_assembler::internal::TypeCmpMacro>();
    });
    macroRegistry->registerMacroName("LogicalAnd", []() {
        return std::make_shared<lyric_assembler::internal::LogicalAndMacro>();
    });
    macroRegistry->registerMacroName("LogicalOr", []() {
        return std::make_shared<lyric_assembler::internal::LogicalOrMacro>();
    });
    macroRegistry->registerMacroName("LogicalNot", []() {
        return std::make_shared<lyric_assembler::internal::LogicalNotMacro>();
    });
    macroRegistry->registerMacroName("BitwiseAnd", []() {
        return std::make_shared<lyric_assembler::internal::BitwiseAndMacro>();
    });
    macroRegistry->registerMacroName("BitwiseOr", []() {
        return std::make_shared<lyric_assembler::internal::BitwiseOrMacro>();
    });
    macroRegistry->registerMacroName("BitwiseXor", []() {
        return std::make_shared<lyric_assembler::internal::BitwiseXorMacro>();
    });
    macroRegistry->registerMacroName("BitwiseLeftShift", []() {
        return std::make_shared<lyric_assembler::internal::BitwiseLeftShiftMacro>();
    });
    macroRegistry->registerMacroName("BitwiseRightShift", []() {
        return std::make_shared<lyric_assembler::internal::BitwiseRightShiftMacro>();
    });

    // compiler macros

    macroRegistry->registerMacroName("PushResult", []() {
        return std::make_shared<lyric_compiler::internal::PushResultMacro>();
    });
    macroRegistry->registerMacroName("PopResult", []() {
        return std::make_shared<lyric_compiler::internal::PopResultMacro>();
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
