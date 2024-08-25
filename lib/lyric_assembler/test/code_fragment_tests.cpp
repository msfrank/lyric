#include <gtest/gtest.h>

#include <lyric_assembler/proc_builder.h>
#include <tempo_test/status_matchers.h>
#include "lyric_bootstrap/bootstrap_loader.h"

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
    lyric_assembler::ProcBuilder procBuilder(&procHandle);
    auto *root = procBuilder.rootFragment();

    ASSERT_THAT (root->immediateNil(), tempo_test::IsOk());

    lyric_object::BytecodeBuilder bytecodeBuilder;
    ASSERT_THAT (procBuilder.build(bytecodeBuilder), tempo_test::IsOk());

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
    lyric_assembler::ProcBuilder procBuilder(&procHandle);
    auto *root = procBuilder.rootFragment();

    ASSERT_THAT (root->appendLabel("top"), tempo_test::IsOk());
    ASSERT_THAT (root->noOperation(), tempo_test::IsOk());
    tu_uint32 targetId;
    TU_ASSIGN_OR_RAISE (targetId, root->unconditionalJump());
    ASSERT_THAT (root->patchTarget(targetId, "top"), tempo_test::IsOk());

    lyric_object::BytecodeBuilder bytecodeBuilder;
    ASSERT_THAT (procBuilder.build(bytecodeBuilder), tempo_test::IsOk());

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
