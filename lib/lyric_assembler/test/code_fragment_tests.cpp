#include <gtest/gtest.h>

#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/object_state.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_importer/module_cache.h>
#include <tempo_test/status_matchers.h>
#include <tempo_tracing/scope_manager.h>
#include <tempo_tracing/trace_recorder.h>

TEST(CodeFragment, ImmediateNil)
{
    auto location = lyric_common::ModuleLocation::fromString("/test");
    auto loader = std::make_shared<lyric_bootstrap::BootstrapLoader>(LYRIC_BUILD_BOOTSTRAP_DIR);
    auto systemModuleCache = lyric_importer::ModuleCache::create(loader);
    auto recorder = tempo_tracing::TraceRecorder::create();
    tempo_tracing::ScopeManager scopeManager(recorder);
    lyric_assembler::ObjectState objectState(location, systemModuleCache, &scopeManager);

    auto activationUrl = lyric_common::SymbolUrl::fromString("#sym");
    lyric_assembler::ProcHandle procHandle(activationUrl, &objectState);
    auto *procCode = procHandle.procCode();
    auto *root = procCode->rootFragment();

    ASSERT_THAT (root->immediateNil(), tempo_test::IsOk());

    lyric_object::BytecodeBuilder bytecodeBuilder;
    ASSERT_THAT (procCode->build(bytecodeBuilder), tempo_test::IsOk());

    auto bytecode = bytecodeBuilder.getBytecode();
    lyric_object::BytecodeIterator it(bytecode.data(), bytecode.size());
    lyric_object::OpCell op;

    ASSERT_TRUE (it.getNext(op));
    ASSERT_EQ (lyric_object::Opcode::OP_NIL, op.opcode);
    ASSERT_EQ (lyric_object::OpInfoType::NO_OPERANDS, op.type);
    ASSERT_FALSE (it.hasNext());
}

TEST(CodeFragment, UnconditionalJump)
{
    auto location = lyric_common::ModuleLocation::fromString("/test");
    auto loader = std::make_shared<lyric_bootstrap::BootstrapLoader>(LYRIC_BUILD_BOOTSTRAP_DIR);
    auto systemModuleCache = lyric_importer::ModuleCache::create(loader);
    auto recorder = tempo_tracing::TraceRecorder::create();
    tempo_tracing::ScopeManager scopeManager(recorder);
    lyric_assembler::ObjectState objectState(location, systemModuleCache, &scopeManager);

    auto activationUrl = lyric_common::SymbolUrl::fromString("#sym");
    lyric_assembler::ProcHandle procHandle(activationUrl, &objectState);
    auto *procCode = procHandle.procCode();
    auto *root = procCode->rootFragment();

    lyric_assembler::JumpLabel label;
    TU_ASSIGN_OR_RAISE (label, root->appendLabel("top"));

    ASSERT_THAT (root->noOperation(), tempo_test::IsOk());
    lyric_assembler::JumpTarget target;
    TU_ASSIGN_OR_RAISE (target, root->unconditionalJump());
    ASSERT_THAT (root->patchTarget(target, label), tempo_test::IsOk());

    lyric_object::BytecodeBuilder bytecodeBuilder;
    ASSERT_THAT (procCode->build(bytecodeBuilder), tempo_test::IsOk());

    auto bytecode = bytecodeBuilder.getBytecode();
    lyric_object::BytecodeIterator it(bytecode.data(), bytecode.size());
    lyric_object::OpCell op;

    ASSERT_TRUE (it.getNext(op));
    ASSERT_EQ (lyric_object::Opcode::OP_NOOP, op.opcode);
    ASSERT_EQ (lyric_object::OpInfoType::NO_OPERANDS, op.type);

    ASSERT_TRUE (it.getNext(op));
    ASSERT_EQ (lyric_object::Opcode::OP_JUMP, op.opcode);
    ASSERT_EQ (lyric_object::OpInfoType::JUMP_I16, op.type);
    ASSERT_EQ (-4, op.operands.jump_i16.jump);

    ASSERT_FALSE (it.hasNext());
}
