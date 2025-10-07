
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