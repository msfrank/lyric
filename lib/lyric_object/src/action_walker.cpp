
#include <lyric_object/action_walker.h>
#include <lyric_object/concept_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/parameter_walker.h>
#include <lyric_object/symbol_walker.h>
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

bool
lyric_object::ActionWalker::isDeclOnly() const
{
    if (!isValid())
        return false;
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return false;
    return bool(actionDescriptor->flags() & lyo1::ActionFlags::DeclOnly);
}

lyric_object::AccessType
lyric_object::ActionWalker::getAccess() const
{
    if (!isValid())
        return AccessType::Invalid;
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return AccessType::Invalid;

    if (bool(actionDescriptor->flags() & lyo1::ActionFlags::GlobalVisibility))
        return AccessType::Public;
    if (bool(actionDescriptor->flags() & lyo1::ActionFlags::InheritVisibility))
        return AccessType::Protected;
    return AccessType::Private;
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
    auto symbolPath = m_reader->getSymbolPath(section, index);
    return SymbolWalker(m_reader, m_reader->getSymbolIndex(symbolPath));
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
lyric_object::ActionWalker::numListParameters() const
{
    if (!isValid())
        return false;
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    if (actionDescriptor->list_parameters() == nullptr)
        return 0;
    return actionDescriptor->list_parameters()->size();
}

lyric_object::ParameterWalker
lyric_object::ActionWalker::getListParameter(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    if (actionDescriptor->list_parameters() == nullptr)
        return {};
    auto *listParameters = actionDescriptor->list_parameters();

    if (listParameters->size() <= index)
        return {};
    auto *parameter = listParameters->Get(index);
    auto placement = parameter->initializer_call() == INVALID_ADDRESS_U32? PlacementType::List
        : PlacementType::ListOpt;

    return ParameterWalker(m_reader, (void *) parameter, index, placement);
}

tu_uint8
lyric_object::ActionWalker::numNamedParameters() const
{
    if (!isValid())
        return false;
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    if (actionDescriptor->named_parameters() == nullptr)
        return 0;
    return actionDescriptor->named_parameters()->size();
}

lyric_object::ParameterWalker
lyric_object::ActionWalker::getNamedParameter(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    if (actionDescriptor->named_parameters() == nullptr)
        return {};
    auto *namedParameters = actionDescriptor->named_parameters();

    if (namedParameters->size() <= index)
        return {};
    auto *parameter = namedParameters->Get(index);

    PlacementType placement = PlacementType::Invalid;
    if (bool(parameter->flags() & lyo1::ParameterFlags::Ctx)) {
        placement = PlacementType::Ctx;
    } else if (parameter->initializer_call() != INVALID_ADDRESS_U32) {
        placement = PlacementType::NamedOpt;
    } else {
        placement = PlacementType::Named;
    }

    return ParameterWalker(m_reader, (void *) parameter, index, placement);
}

bool
lyric_object::ActionWalker::hasRestParameter() const
{
    if (!isValid())
        return false;
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    return actionDescriptor->rest_parameter() != nullptr;
}

lyric_object::ParameterWalker
lyric_object::ActionWalker::getRestParameter() const
{
    if (!isValid())
        return {};
    auto *actionDescriptor = m_reader->getAction(m_actionOffset);
    if (actionDescriptor == nullptr)
        return {};
    auto *parameter =  actionDescriptor->rest_parameter();
    if (parameter == nullptr)
        return {};
    return ParameterWalker(m_reader, (void *) parameter, 0, PlacementType::Rest);
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
