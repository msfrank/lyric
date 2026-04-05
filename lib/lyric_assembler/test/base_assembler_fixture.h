#ifndef LYRIC_ASSEMBLER_BASE_ASSEMBLER_FIXTURE_H
#define LYRIC_ASSEMBLER_BASE_ASSEMBLER_FIXTURE_H

#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>

#include "lyric_assembler/object_state.h"

class BaseAssemblerFixture : public ::testing::Test {
protected:
    std::unique_ptr<lyric_assembler::ObjectState> objectState;
    lyric_assembler::ObjectRoot *objectRoot;

    void SetUp() override;

    tempo_utils::Status writeObject(lyric_object::LyricObject &object);
    tempo_utils::Status writeObjectWithEmptyEntry(lyric_object::LyricObject &object);
};

#endif // LYRIC_ASSEMBLER_BASE_ASSEMBLER_FIXTURE_H
