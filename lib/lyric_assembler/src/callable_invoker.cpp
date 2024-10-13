
#include <lyric_assembler/assembler_result.h>
#include <lyric_assembler/callable_invoker.h>

lyric_assembler::CallableInvoker::CallableInvoker()
{
}

tempo_utils::Status
lyric_assembler::CallableInvoker::initialize(std::unique_ptr<AbstractCallable> &&callable)
{
    if (m_callable != nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "callable was already initialized");
    m_callable = std::move(callable);
    return {};
}

const lyric_assembler::AbstractCallable *
lyric_assembler::CallableInvoker::getCallable() const
{
    return m_callable.get();
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::CallableInvoker::invoke(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier,
    CodeFragment *fragment)
{
    if (m_callable == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "callable is not initialized");
    return m_callable->invoke(block, reifier, fragment);
}