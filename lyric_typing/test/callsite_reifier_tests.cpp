#include <gtest/gtest.h>

#include <lyric_typing/callsite_reifier.h>

#include "base_fixture.h"

class CallsiteReifier : public BaseFixture {};

TEST_F(CallsiteReifier, ReifierWithNoTypeParametersIsValid)
{
    lyric_typing::CallsiteReifier reifier({}, {}, m_typeSystem.get());
    ASSERT_TRUE (reifier.isValid());
    ASSERT_EQ (0, reifier.numArguments());
}
