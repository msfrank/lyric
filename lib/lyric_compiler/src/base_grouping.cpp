
#include <lyric_compiler/base_choice.h>
#include <lyric_compiler/base_grouping.h>
#include <lyric_compiler/compiler_result.h>

lyric_compiler::BaseGrouping::BaseGrouping(CompilerScanDriver *driver)
    : m_block(nullptr),
      m_driver(driver),
      m_curr(0)
{
    TU_ASSERT (m_driver != nullptr);
}

lyric_compiler::BaseGrouping::BaseGrouping(
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : m_block(block),
      m_driver(driver),
      m_curr(0)
{
    TU_ASSERT (m_block != nullptr);
    TU_ASSERT (m_driver != nullptr);
}

tempo_utils::Status
lyric_compiler::BaseGrouping::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    return {};
}

tempo_utils::Status
lyric_compiler::BaseGrouping::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    return {};
}

lyric_assembler::BlockHandle *
lyric_compiler::BaseGrouping::getBlock()
{
    return m_block;
}

lyric_compiler::CompilerScanDriver *
lyric_compiler::BaseGrouping::getDriver()
{
    return m_driver;
}

lyric_compiler::Handler *
lyric_compiler::BaseGrouping::currentHandler() const
{
    if (isFinished())
        return nullptr;
    auto &handler = m_handlers.at(m_curr);
    return handler.get();
}

void
lyric_compiler::BaseGrouping::appendHandlers(std::vector<std::unique_ptr<Handler>> &&handlers)
{
    for (int i = 0; i < handlers.size(); i++) {
        auto handler = std::move(handlers[i]);
        m_handlers.push_back(std::move(handler));
    }
}

bool
lyric_compiler::BaseGrouping::advanceHandler()
{
    m_curr++;
    return m_handlers.size() <= m_curr;
}

bool
lyric_compiler::BaseGrouping::isFinished() const
{
    return m_handlers.size() <= m_curr;
}

//tempo_utils::Status
//lyric_compiler::BaseGrouping::enter(
//    const lyric_parser::ArchetypeState *state,
//    const lyric_parser::ArchetypeNode *node,
//    EnterContext &ctx)
//{
//    if (m_curr < m_handlers.size()) {
//        auto &handler = m_handlers.at(m_curr);
//        switch (handler->type) {
//            case HandlerType::Behavior: {
//                auto &behavior = handler->behavior;
//                return behavior->enter(m_driver, state, node, m_block, ctx);
//            }
//            case HandlerType::Choice: {
//                auto &choice = handler->choice;
//                break;
//            }
//            case HandlerType::Grouping: {
//                auto &grouping = handler->grouping;
//                break;
//            }
//            default:
//                return CompilerStatus::forCondition(
//                    CompilerCondition::kCompilerInvariant, "invalid handler for grouping");
//        }
//    }
//    return CompilerStatus::forCondition(
//        CompilerCondition::kCompilerInvariant, "no behavior available for node");
//}
//
//tempo_utils::Status
//lyric_compiler::BaseGrouping::exit(
//    const lyric_parser::ArchetypeState *state,
//    const lyric_parser::ArchetypeNode *node,
//    ExitContext &ctx)
//{
//    auto &behavior = m_behaviors.at(m_curr);
//    TU_RETURN_IF_NOT_OK (behavior->exit(m_driver, state, node, m_block, ctx));
//    m_curr++;
//    if (m_curr == m_behaviors.size()) {
//        ctx.setPopHandler(true);
//    }
//    return {};
//}