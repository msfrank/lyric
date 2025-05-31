
#include <lyric_compiler/abstract_behavior.h>
#include <lyric_compiler/base_choice.h>
#include <lyric_compiler/base_grouping.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/visitor_context.h>

lyric_compiler::BeforeContext::BeforeContext(
    lyric_rewriter::VisitorContext &visitorContext,
    const CompilerScanDriver *driver)
    : m_visitorContext(visitorContext),
      m_driver(driver)
{
    TU_ASSERT (m_driver != nullptr);
}

const lyric_compiler::CompilerScanDriver *
lyric_compiler::BeforeContext::getDriver() const
{
    return m_driver;
}

const lyric_parser::ArchetypeNode *
lyric_compiler::BeforeContext::parentNode() const
{
    return m_visitorContext.parentNode();
}

int
lyric_compiler::BeforeContext::childIndex() const
{
    return m_visitorContext.childIndex();
}

void
lyric_compiler::BeforeContext::setSkipChildren(bool skip)
{
    m_visitorContext.setSkipChildren(skip);
}

/**
 * Appends the specified `behavior` to the behaviors list of the current handler.
 *
 * @param behavior The behavior.
 */
void
lyric_compiler::BeforeContext::appendBehavior(std::unique_ptr<AbstractBehavior> &&behavior)
{
    TU_ASSERT (behavior != nullptr);
    auto handler = std::make_unique<Handler>();
    handler->type = HandlerType::Behavior;
    handler->behavior = std::move(behavior);
    m_handlers.push_back(std::move(handler));
}

void
lyric_compiler::BeforeContext::appendChoice(std::unique_ptr<BaseChoice> &&choice)
{
    TU_ASSERT (choice != nullptr);
    auto handler = std::make_unique<Handler>();
    handler->type = HandlerType::Choice;
    handler->choice = std::move(choice);
    m_handlers.push_back(std::move(handler));
}

void
lyric_compiler::BeforeContext::appendGrouping(std::unique_ptr<BaseGrouping> &&grouping)
{
    TU_ASSERT (grouping != nullptr);
    auto handler = std::make_unique<Handler>();
    handler->type = HandlerType::Grouping;
    handler->grouping = std::move(grouping);
    m_handlers.push_back(std::move(handler));
}

std::vector<std::unique_ptr<lyric_compiler::Handler>>&&
lyric_compiler::BeforeContext::takeHandlers()
{
    return std::move(m_handlers);
}

lyric_compiler::AfterContext::AfterContext(
    const lyric_rewriter::VisitorContext &visitorContext,
    const CompilerScanDriver *driver)
    : m_visitorContext(visitorContext),
      m_driver(driver)
{
    TU_ASSERT (m_driver != nullptr);
}

const lyric_compiler::CompilerScanDriver *
lyric_compiler::AfterContext::getDriver() const
{
    return m_driver;
}

const lyric_parser::ArchetypeNode *
lyric_compiler::AfterContext::parentNode() const
{
    return m_visitorContext.parentNode();
}

int
lyric_compiler::AfterContext::childIndex() const
{
    return m_visitorContext.childIndex();
}

lyric_compiler::EnterContext::EnterContext(
    lyric_rewriter::VisitorContext &visitorContext,
    const CompilerScanDriver *driver)
    : m_visitorContext(visitorContext),
      m_driver(driver)
{
    TU_ASSERT (m_driver != nullptr);
}

const lyric_compiler::CompilerScanDriver *
lyric_compiler::EnterContext::getDriver() const
{
    return m_driver;
}

const lyric_parser::ArchetypeNode *
lyric_compiler::EnterContext::parentNode() const
{
    return m_visitorContext.parentNode();
}

int
lyric_compiler::EnterContext::childIndex() const
{
    return m_visitorContext.childIndex();
}

void
lyric_compiler::EnterContext::appendBehavior(std::unique_ptr<AbstractBehavior> &&behavior)
{
    TU_ASSERT (behavior != nullptr);
    auto handler = std::make_unique<Handler>();
    handler->type = HandlerType::Behavior;
    handler->behavior = std::move(behavior);
    m_handlers.push_back(std::move(handler));
}

void
lyric_compiler::EnterContext::appendChoice(std::unique_ptr<BaseChoice> &&choice)
{
    TU_ASSERT (choice != nullptr);
    auto handler = std::make_unique<Handler>();
    handler->type = HandlerType::Choice;
    handler->choice = std::move(choice);
    m_handlers.push_back(std::move(handler));
}

void
lyric_compiler::EnterContext::appendGrouping(std::unique_ptr<BaseGrouping> &&grouping)
{
    TU_ASSERT (grouping != nullptr);
    auto handler = std::make_unique<Handler>();
    handler->type = HandlerType::Grouping;
    handler->grouping = std::move(grouping);
    m_handlers.push_back(std::move(handler));
}

std::vector<std::unique_ptr<lyric_compiler::Handler>>&&
lyric_compiler::EnterContext::takeHandlers()
{
    return std::move(m_handlers);
}

lyric_compiler::ExitContext::ExitContext(
    const lyric_rewriter::VisitorContext &visitorContext,
    const CompilerScanDriver *driver)
    : m_visitorContext(visitorContext),
      m_driver(driver)
{
    TU_ASSERT (m_driver != nullptr);
}

const lyric_compiler::CompilerScanDriver *
lyric_compiler::ExitContext::getDriver() const
{
    return m_driver;
}

const lyric_parser::ArchetypeNode *
lyric_compiler::ExitContext::parentNode() const
{
    return m_visitorContext.parentNode();
}

int
lyric_compiler::ExitContext::childIndex() const
{
    return m_visitorContext.childIndex();
}

bool
lyric_compiler::ExitContext::popHandler() const
{
    return m_popHandler;
}

void
lyric_compiler::ExitContext::setPopHandler(bool popHandler)
{
    m_popHandler = popHandler;
}

lyric_compiler::DecideContext::DecideContext(
    const lyric_rewriter::VisitorContext &visitorContext,
    const CompilerScanDriver *driver)
    : m_visitorContext(visitorContext),
      m_driver(driver)
{
    TU_ASSERT (m_driver != nullptr);
}

const lyric_compiler::CompilerScanDriver *
lyric_compiler::DecideContext::getDriver() const
{
    return m_driver;
}

const lyric_parser::ArchetypeNode *
lyric_compiler::DecideContext::parentNode() const
{
    return m_visitorContext.parentNode();
}

int
lyric_compiler::DecideContext::childIndex() const
{
    return m_visitorContext.childIndex();
}

void
lyric_compiler::DecideContext::setBehavior(std::unique_ptr<AbstractBehavior> &&behavior)
{
    m_handler = std::make_unique<Handler>();
    m_handler->type = HandlerType::Behavior;
    m_handler->behavior = std::move(behavior);
}

void
lyric_compiler::DecideContext::setChoice(std::unique_ptr<BaseChoice> &&choice)
{
    m_handler = std::make_unique<Handler>();
    m_handler->type = HandlerType::Choice;
    m_handler->choice = std::move(choice);
}

void
lyric_compiler::DecideContext::setGrouping(std::unique_ptr<BaseGrouping> &&grouping)
{
    m_handler = std::make_unique<Handler>();
    m_handler->type = HandlerType::Grouping;
    m_handler->grouping = std::move(grouping);
}

std::unique_ptr<lyric_compiler::Handler>&&
lyric_compiler::DecideContext::takeHandler()
{
    return std::move(m_handler);
}