
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_module.h>
#include <lyric_compiler/internal/compile_node.h>
#include <tempo_utils/log_stream.h>

tempo_utils::Result<lyric_object::LyricObject>
lyric_compiler::internal::compile_module(
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry,
    bool touchExternalSymbols)
{

    auto *root = moduleEntry.getRoot();
    auto *rootBlock = root->namespaceBlock();

    // compile assembly starting at entry node
    auto result = compile_node(rootBlock, walker, moduleEntry);
    if (result.isStatus())
        return result.getStatus();
    auto *entry = moduleEntry.getEntry();
    entry->putExitType(result.getResult());

    // add return instruction
    // FIXME: a jump offset which points to the end of the proc requires a pad
    auto *proc = entry->callProc();
    auto status = proc->procCode()->writeOpcode(lyric_object::Opcode::OP_RETURN);
    if (status.notOk())
        return status;

//    // if entry contains code, then touch the symbol so it gets serialized
//    auto *proc = moduleEntry.getProc();
//    if (proc->procCode()->bytecodeSize() > 0) {
//        moduleEntry.touch();
//    }

    auto *state = moduleEntry.getState();

    // touch all external symbols defined in the entry block so they are added to links
    if (touchExternalSymbols) {
        auto preludeLocation = state->fundamentalCache()->getPreludeLocation();
        auto *procBlock = entry->callProc()->procBlock();
        for (auto iterator = procBlock->symbolsBegin(); iterator != procBlock->symbolsEnd(); iterator++) {
            auto symbolUrl = iterator->second.symbolUrl;
            // ignore core symbols since they are guaranteed to be present at runtime
            if (symbolUrl.getAssemblyLocation() == preludeLocation)
                continue;
            // ignore relative symbols since these are not external
            if (symbolUrl.isRelative())
                continue;
            state->symbolCache()->touchSymbol(symbolUrl);
        }
    }

    // construct assembly from assembly state and return it
    auto toAssemblyResult = state->toAssembly();
    if (toAssemblyResult.isStatus())
        return toAssemblyResult.getStatus();
    return toAssemblyResult.getResult();
}