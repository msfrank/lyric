#ifndef LYRIC_TYPING_BASE_FIXTURE_H
#define LYRIC_TYPING_BASE_FIXTURE_H

#include <gtest/gtest.h>

#include <lyric_assembler/assembly_state.h>
#include <lyric_typing/type_system.h>
#include <tempo_tracing/scope_manager.h>

class BaseFixture : public ::testing::Test {
protected:
    std::unique_ptr<tempo_tracing::ScopeManager> m_scopeManager;
    std::unique_ptr<lyric_assembler::AssemblyState> m_assemblyState;
    std::unique_ptr<lyric_typing::TypeSystem> m_typeSystem;

    void SetUp() override;
};

#endif // LYRIC_TYPING_BASE_FIXTURE_H