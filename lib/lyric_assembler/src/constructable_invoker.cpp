
#include <lyric_assembler/assembler_result.h>
#include <lyric_assembler/constructable_invoker.h>

lyric_assembler::ConstructableInvoker::ConstructableInvoker()
{
}

tempo_utils::Status
lyric_assembler::ConstructableInvoker::initialize(std::unique_ptr<AbstractConstructable> &&constructable)
{
    if (m_constructable != nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "constructable was already initialized");
    m_constructable = std::move(constructable);
    return {};
}

const lyric_assembler::AbstractConstructable *
lyric_assembler::ConstructableInvoker::getConstructable() const
{
    return m_constructable.get();
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::ConstructableInvoker::invoke(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier,
    tu_uint8 flags)
{
    if (m_constructable == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "constructable is not initialized");
    return m_constructable->invoke(block, reifier, flags);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::ConstructableInvoker::invokeNew(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier,
    tu_uint8 flags)
{
    if (m_constructable == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "constructable is not initialized");
    return m_constructable->invokeNew(block, reifier, flags);
}
