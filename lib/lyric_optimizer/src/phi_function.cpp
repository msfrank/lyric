
#include <lyric_optimizer/internal/cfg_data.h>
#include <lyric_optimizer/optimizer_result.h>
#include <lyric_optimizer/phi_function.h>

lyric_optimizer::PhiFunction::PhiFunction()
{
}

lyric_optimizer::PhiFunction::PhiFunction(const PhiFunction &other)
    : m_phi(other.m_phi)
{
}

lyric_optimizer::PhiFunction::PhiFunction(std::shared_ptr<internal::PhiPriv> phi)
    : m_phi(phi)
{
    TU_ASSERT (m_phi != nullptr);
}

bool
lyric_optimizer::PhiFunction::isValid() const
{
    return m_phi != nullptr;
}

lyric_optimizer::Instance
lyric_optimizer::PhiFunction::getPhiTarget() const
{
    if (m_phi == nullptr)
        return {};
    return m_phi->target;
}

std::vector<lyric_optimizer::Instance>::const_iterator
lyric_optimizer::PhiFunction::phiArgumentsBegin() const
{
    if (m_phi == nullptr)
        return {};
    return m_phi->arguments.cbegin();
}

std::vector<lyric_optimizer::Instance>::const_iterator
lyric_optimizer::PhiFunction::phiArgumentsEnd() const
{
    if (m_phi == nullptr)
        return {};
    return m_phi->arguments.cend();
}

int
lyric_optimizer::PhiFunction::numPhiArguments() const
{
    if (m_phi == nullptr)
        return 0;
    return m_phi->arguments.size();
}