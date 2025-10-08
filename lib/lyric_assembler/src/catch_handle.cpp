
#include <lyric_assembler/catch_handle.h>

lyric_assembler::CatchHandle::CatchHandle(
    const lyric_common::TypeDef &exceptionType,
    CodeFragment *fragment,
    ObjectState *state)
    : m_exceptionType(exceptionType),
      m_fragment(std::move(fragment)),
      m_state(state)
{
    TU_ASSERT (m_exceptionType.isValid());
    TU_ASSERT (m_fragment != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_common::TypeDef
lyric_assembler::CatchHandle::getExceptionType() const
{
    return m_exceptionType;
}

lyric_assembler::JumpTarget
lyric_assembler::CatchHandle::getResumeTarget() const
{
    return m_resumeTarget;
}

lyric_assembler::CodeFragment *
lyric_assembler::CatchHandle::catchFragment() const
{
    return m_fragment;
}

lyric_assembler::ObjectState *
lyric_assembler::CatchHandle::objectState() const
{
    return m_state;
}

tempo_utils::Status
lyric_assembler::CatchHandle::finalizeCatch()
{
    auto numStatements = m_fragment->numStatements();
    bool unfinished = true;

    if (numStatements > 0) {
        auto lastStatement = m_fragment->getStatement(numStatements - 1);
        switch (lastStatement.instruction->getType()) {
            case InstructionType::Jump:
            case InstructionType::Return:
            case InstructionType::Interrupt:
            case InstructionType::Abort:
            case InstructionType::Halt:
                unfinished = false;
                break;
            default:
                break;
        }
    }

    if (unfinished) {
        TU_ASSIGN_OR_RETURN (m_resumeTarget, m_fragment->unconditionalJump());
    }

    return {};
}