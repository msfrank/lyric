#ifndef LYRIC_TYPING_TEST_CALLABLE_H
#define LYRIC_TYPING_TEST_CALLABLE_H

#include <lyric_assembler/abstract_callable.h>
#include <lyric_assembler/abstract_callsite_reifier.h>
#include <lyric_assembler/block_handle.h>

class TestCallable : public lyric_assembler::AbstractCallable {
public:
    TestCallable(
        const std::vector<lyric_assembler::Parameter> &listParameters,
        const std::vector<lyric_assembler::Parameter> &namedParameters,
        const lyric_assembler::Parameter &restParameter);
    TestCallable(
        const std::vector<lyric_assembler::Parameter> &listParameters,
        const std::vector<lyric_assembler::Parameter> &namedParameters,
        const lyric_assembler::Parameter &restParameter,
        lyric_assembler::TemplateHandle *templateHandle);

    lyric_assembler::TemplateHandle *getTemplate() const override;
    std::vector<lyric_assembler::Parameter>::const_iterator listPlacementBegin() const override;
    std::vector<lyric_assembler::Parameter>::const_iterator listPlacementEnd() const override;
    std::vector<lyric_assembler::Parameter>::const_iterator namedPlacementBegin() const override;
    std::vector<lyric_assembler::Parameter>::const_iterator namedPlacementEnd() const override;
    const lyric_assembler::Parameter *restPlacement() const override;
    bool hasInitializer(const std::string &name) const override;
    lyric_common::SymbolUrl getInitializer(const std::string &name) const override;

    tempo_utils::Result<lyric_common::TypeDef> invoke(
        lyric_assembler::BlockHandle *block,
        const lyric_assembler::AbstractCallsiteReifier &reifier) override;

private:
    lyric_assembler::TemplateHandle *m_templateHandle;
    std::vector<lyric_assembler::Parameter> m_listParameters;
    std::vector<lyric_assembler::Parameter> m_namedParameters;
    lyric_assembler::Parameter m_restParameter;
};

#endif // LYRIC_TYPING_TEST_CALLABLE_H
