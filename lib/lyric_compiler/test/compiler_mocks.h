#ifndef LYRIC_COMPILER_COMPILER_MOCKS_H
#define LYRIC_COMPILER_COMPILER_MOCKS_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <lyric_compiler/base_choice.h>
#include <lyric_compiler/base_grouping.h>

class MockBehavior : public lyric_compiler::AbstractBehavior {
public:

    MOCK_METHOD (tempo_utils::Status, enter, (
        lyric_compiler::CompilerScanDriver *,
    const lyric_parser::ArchetypeState *,
    const lyric_parser::ArchetypeNode *,
    lyric_assembler::BlockHandle *,
    lyric_compiler::EnterContext &),
    (override));

    MOCK_METHOD (tempo_utils::Status, exit, (
        lyric_compiler::CompilerScanDriver *,
    const lyric_parser::ArchetypeState *,
    const lyric_parser::ArchetypeNode *,
    lyric_assembler::BlockHandle *,
    lyric_compiler::ExitContext &),
    (override));
};

class MockChoice : public lyric_compiler::BaseChoice {
public:
    MockChoice(lyric_compiler::CompilerScanDriver *driver)
        : lyric_compiler::BaseChoice(driver)
    {
    }

    MOCK_METHOD (tempo_utils::Status, decide, (
        const lyric_parser::ArchetypeState *,
        const lyric_parser::ArchetypeNode *,
        lyric_compiler::DecideContext &),
                 (override));
};

class MockGrouping : public lyric_compiler::BaseGrouping {
public:
    MockGrouping(lyric_compiler::CompilerScanDriver *driver)
        : lyric_compiler::BaseGrouping(driver)
    {
    }

    MOCK_METHOD (tempo_utils::Status, before, (
    const lyric_parser::ArchetypeState *,
    const lyric_parser::ArchetypeNode *,
    lyric_compiler::BeforeContext &),
    (override));

    MOCK_METHOD (tempo_utils::Status, after, (
    const lyric_parser::ArchetypeState *,
    const lyric_parser::ArchetypeNode *,
    lyric_compiler::AfterContext &),
    (override));
};

#endif // LYRIC_COMPILER_COMPILER_MOCKS_H
