#ifndef LYRIC_TYPING_BASE_FIXTURE_H
#define LYRIC_TYPING_BASE_FIXTURE_H

#include <gtest/gtest.h>

#include <lyric_assembler/object_root.h>
#include <lyric_assembler/object_state.h>
#include <lyric_typing/type_system.h>

class BaseTypingFixture : public ::testing::Test {
protected:
    lyric_common::ModuleLocation location;
    std::unique_ptr<lyric_assembler::ObjectState> objectState;
    lyric_assembler::ObjectRoot *objectRoot;
    std::unique_ptr<lyric_typing::TypeSystem> typeSystem;

    void SetUp() override;
};

#endif // LYRIC_TYPING_BASE_FIXTURE_H