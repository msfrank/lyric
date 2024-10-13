
#include <lyric_compiler/binary_operation_handler.h>
#include <lyric_compiler/form_handler.h>
#include <lyric_compiler/operator_utils.h>

lyric_compiler::BinaryOperationHandler::BinaryOperationHandler(
    lyric_schema::LyricAstId astId,
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_astId(astId),
      m_isSideEffect(isSideEffect),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::BinaryOperationHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::BeforeContext &ctx)
{
    TU_LOG_INFO << "before BinaryOperationHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto op1 = std::make_unique<FormChoice>(FormType::Expression, m_fragment, block, driver);
    ctx.appendChoice(std::move(op1));
    auto op2 = std::make_unique<FormChoice>(FormType::Expression, m_fragment, block, driver);
    ctx.appendChoice(std::move(op2));
    return {};
}

tempo_utils::Status
lyric_compiler::BinaryOperationHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    TU_LOG_INFO << "after BinaryOperationHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();

    TU_RETURN_IF_NOT_OK (compile_binary_operator(m_astId, block, m_fragment, driver));

    if (m_isSideEffect) {
        auto resultType = driver->peekResult();
        TU_RETURN_IF_NOT_OK (driver->popResult());
        if (resultType.getType() != lyric_common::TypeDefType::NoReturn) {
            TU_RETURN_IF_NOT_OK (m_fragment->popValue());
        }
    }

    return {};
}