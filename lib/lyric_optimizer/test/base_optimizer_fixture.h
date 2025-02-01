#ifndef BASE_OPTIMIZER_FIXTURE_H
#define BASE_OPTIMIZER_FIXTURE_H

#include <gtest/gtest.h>

#include <lyric_assembler/object_root.h>
#include <lyric_assembler/object_state.h>
#include <lyric_optimizer/lyric_optimizer.h>
#include <lyric_runtime/static_loader.h>
#include <lyric_test/lyric_tester.h>

class BaseOptimizerFixture : public ::testing::Test {
protected:
    BaseOptimizerFixture();

    tempo_utils::Status configure();

    tempo_utils::Result<lyric_assembler::CallSymbol *> declareFunction(
        const std::string &name,
        lyric_object::AccessType access,
        const std::vector<lyric_object::TemplateParameter> &templateParameters = {});

    std::shared_ptr<lyric_runtime::StaticLoader> m_staticLoader;
    lyric_test::TesterOptions m_testerOptions;
    std::unique_ptr<lyric_test::LyricTester> m_tester;
    std::unique_ptr<lyric_assembler::ObjectState> m_objectState;
    lyric_assembler::ObjectRoot *m_objectRoot;
};


#endif // BASE_OPTIMIZER_FIXTURE_H
