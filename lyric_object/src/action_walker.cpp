
#include <lyric_object/action_parameter_walker.h>
#include <lyric_object/action_walker.h>
#include <lyric_object/concept_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/symbol_walker.h>
#include <lyric_object/rest_parameter_walker.h>
#include <lyric_object/template_walker.h>
#include <lyric_object/type_walker.h>
#include <tempo_utils/big_endian.h>

lyric_object::ActionWalker::ActionWalker()
    : m_actionOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::ActionWalker::ActionWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 actionOffset)
    : m_reader(reader),
      m_actionOffset(actionOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::ActionWalker::ActionWalker(const ActionWalker &other)
    : m_reader(other.m_reader),
      m_actionOffset(other.m_actionOffset)
{
}

bool
lyric_object::ActionWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_actionOffset < m_reader->numActions();
}

lyric_common::SymbolPath
lyric_object::ActionWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    if (actionDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(actionDescriptor->fqsn()->str());
}

lyric_object::SymbolWalker
lyric_object::ActionWalker::getReceiver() const
{

    if (!isValid())
        return {};
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    lyo1::DescriptorSection section;
    switch (actionDescriptor->receiver_section()) {
        case lyo1::TypeSection::Concept:
            section = lyo1::DescriptorSection::Concept;
            break;
        default:
            return {};
    }
    tu_uint32 index = actionDescriptor->receiver_descriptor();
    return SymbolWalker(m_reader, static_cast<tu_uint8>(section), index);
}

bool
lyric_object::ActionWalker::hasTemplate() const
{
    if (!isValid())
        return false;
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    return actionDescriptor->action_template() != INVALID_ADDRESS_U32;
}

lyric_object::TemplateWalker
lyric_object::ActionWalker::getTemplate() const
{
    if (!hasTemplate())
        return {};
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    return TemplateWalker(m_reader, actionDescriptor->action_template());
}

tu_uint8
lyric_object::ActionWalker::numParameters() const
{
    if (!isValid())
        return false;
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    if (actionDescriptor->parameters() == nullptr)
        return 0;
    return actionDescriptor->parameters()->size();
}

lyric_object::ActionParameterWalker
lyric_object::ActionWalker::getParameter(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    if (actionDescriptor->parameters() == nullptr)
        return {};
    if (actionDescriptor->parameters()->size() <= index)
        return {};
    return ActionParameterWalker(m_reader, (void *) actionDescriptor, index);
}

bool
lyric_object::ActionWalker::hasRest() const
{
    if (!isValid())
        return false;
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    return actionDescriptor->rest() != nullptr;
}

lyric_object::RestParameterWalker
lyric_object::ActionWalker::getRest() const
{
    if (!isValid())
        return {};
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    auto *rest =  actionDescriptor->rest();
    if (rest == nullptr)
        return {};
    return RestParameterWalker(m_reader, (void *) rest, (void *) actionDescriptor->names());
}

lyric_object::TypeWalker
lyric_object::ActionWalker::getResultType() const
{
    if (!isValid())
        return {};
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    if (actionDescriptor->result_type() == INVALID_ADDRESS_U32)
        return {};
    return TypeWalker(m_reader, actionDescriptor->result_type());
}

tu_uint32
lyric_object::ActionWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_actionOffset;
}
