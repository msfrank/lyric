
#include <lyric_object/action_walker.h>
#include <lyric_object/concept_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/template_walker.h>
#include <lyric_object/type_walker.h>

lyric_object::ConceptAction::ConceptAction()
    : m_conceptDescriptor(nullptr)
{
}

lyric_object::ConceptAction::ConceptAction(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *conceptDescriptor,
    tu_uint8 actionOffset)
    : m_reader(reader),
      m_conceptDescriptor(conceptDescriptor),
      m_actionOffset(actionOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_conceptDescriptor != nullptr);
}

lyric_object::ConceptAction::ConceptAction(const ConceptAction &other)
    : m_reader(other.m_reader),
      m_conceptDescriptor(other.m_conceptDescriptor),
      m_actionOffset(other.m_actionOffset)
{
}

bool
lyric_object::ConceptAction::isValid() const
{
    return m_reader && m_reader->isValid() && m_conceptDescriptor;
}

//std::string
//lyric_object::ConceptAction::getName() const
//{
//    if (!isValid())
//        return {};
//    auto *conceptDescriptor = static_cast<const lyo1::ConceptDescriptor *>(m_conceptDescriptor);
//    if (conceptDescriptor->actions() == nullptr)
//        return {};
//    if (conceptDescriptor->actions()->size() <= m_actionOffset)
//        return {};
//    auto *conceptAction = conceptDescriptor->actions()->Get(m_actionOffset);
//    if (conceptDescriptor->names() == nullptr)
//        return {};
//    if (conceptDescriptor->names()->size() <= conceptAction->name_offset())
//        return {};
//    return conceptDescriptor->names()->Get(conceptAction->name_offset())->str();
//}

lyric_object::AddressType
lyric_object::ConceptAction::actionAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *conceptDescriptor = static_cast<const lyo1::ConceptDescriptor *>(m_conceptDescriptor);
    if (conceptDescriptor->actions() == nullptr)
        return AddressType::Invalid;
    if (conceptDescriptor->actions()->size() <= m_actionOffset)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(conceptDescriptor->actions()->Get(m_actionOffset));
}

lyric_object::ActionWalker
lyric_object::ConceptAction::getNearAction() const
{
    if (!isValid())
        return {};
    auto *conceptDescriptor = static_cast<const lyo1::ConceptDescriptor *>(m_conceptDescriptor);
    if (conceptDescriptor->actions() == nullptr)
        return {};
    if (conceptDescriptor->actions()->size() <= m_actionOffset)
        return {};
    return ActionWalker(m_reader, GET_DESCRIPTOR_OFFSET(conceptDescriptor->actions()->Get(m_actionOffset)));
}

lyric_object::LinkWalker
lyric_object::ConceptAction::getFarAction() const
{
    if (!isValid())
        return {};
    auto *conceptDescriptor = static_cast<const lyo1::ConceptDescriptor *>(m_conceptDescriptor);
    if (conceptDescriptor->actions() == nullptr)
        return {};
    if (conceptDescriptor->actions()->size() <= m_actionOffset)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(conceptDescriptor->actions()->Get(m_actionOffset)));
}

lyric_object::ConceptWalker::ConceptWalker()
    : m_conceptOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::ConceptWalker::ConceptWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 conceptOffset)
    : m_reader(reader),
      m_conceptOffset(conceptOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::ConceptWalker::ConceptWalker(const ConceptWalker &other)
    : m_reader(other.m_reader),
      m_conceptOffset(other.m_conceptOffset)
{
}

bool
lyric_object::ConceptWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_conceptOffset < m_reader->numConcepts();
}

lyric_common::SymbolPath
lyric_object::ConceptWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *conceptDescriptor = m_reader->getConcept(m_conceptOffset);
    if (conceptDescriptor == nullptr)
        return {};
    if (conceptDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(conceptDescriptor->fqsn()->str());
}

bool
lyric_object::ConceptWalker::isDeclOnly() const
{
    if (!isValid())
        return false;
    auto *conceptDescriptor = m_reader->getConcept(m_conceptOffset);
    if (conceptDescriptor == nullptr)
        return false;
    return bool(conceptDescriptor->flags() & lyo1::ConceptFlags::DeclOnly);
}

lyric_object::DeriveType
lyric_object::ConceptWalker::getDeriveType() const
{
    if (!isValid())
        return DeriveType::Any;
    auto *conceptDescriptor = m_reader->getConcept(m_conceptOffset);
    if (conceptDescriptor == nullptr)
        return DeriveType::Any;
    if (bool(conceptDescriptor->flags() & lyo1::ConceptFlags::Final))
        return DeriveType::Final;
    if (bool(conceptDescriptor->flags() & lyo1::ConceptFlags::Sealed))
        return DeriveType::Sealed;
    return DeriveType::Any;
}

bool
lyric_object::ConceptWalker::hasSuperConcept() const
{
    if (!isValid())
        return false;
    auto *conceptDescriptor = m_reader->getConcept(m_conceptOffset);
    if (conceptDescriptor == nullptr)
        return false;
    return conceptDescriptor->super_concept() != INVALID_ADDRESS_U32;
}

lyric_object::AddressType
lyric_object::ConceptWalker::superConceptAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *conceptDescriptor = m_reader->getConcept(m_conceptOffset);
    if (conceptDescriptor == nullptr)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(conceptDescriptor->super_concept());
}

lyric_object::ConceptWalker
lyric_object::ConceptWalker::getNearSuperConcept() const
{
    if (!hasSuperConcept())
        return {};
    auto *conceptDescriptor = m_reader->getConcept(m_conceptOffset);
    if (conceptDescriptor == nullptr)
        return {};
    return ConceptWalker(m_reader, GET_DESCRIPTOR_OFFSET(conceptDescriptor->super_concept()));
}

lyric_object::LinkWalker
lyric_object::ConceptWalker::getFarSuperConcept() const
{
    if (!hasSuperConcept())
        return {};
    auto *conceptDescriptor = m_reader->getConcept(m_conceptOffset);
    if (conceptDescriptor == nullptr)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(conceptDescriptor->super_concept()));
}

bool
lyric_object::ConceptWalker::hasTemplate() const
{
    if (!isValid())
        return false;
    auto *conceptDescriptor = m_reader->getConcept(m_conceptOffset);
    if (conceptDescriptor == nullptr)
        return false;
    return conceptDescriptor->concept_template() != INVALID_ADDRESS_U32;
}

lyric_object::TemplateWalker
lyric_object::ConceptWalker::getTemplate() const
{
    if (!hasTemplate())
        return {};
    auto *conceptDescriptor = m_reader->getConcept(m_conceptOffset);
    if (conceptDescriptor == nullptr)
        return {};
    return TemplateWalker(m_reader, conceptDescriptor->concept_template());
}

tu_uint8
lyric_object::ConceptWalker::numActions() const
{
    if (!isValid())
        return 0;
    auto *conceptDescriptor = m_reader->getConcept(m_conceptOffset);
    if (conceptDescriptor == nullptr)
        return 0;
    if (conceptDescriptor->actions() == nullptr)
        return 0;
    return conceptDescriptor->actions()->size();
}

lyric_object::ConceptAction
lyric_object::ConceptWalker::getAction(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *conceptDescriptor = m_reader->getConcept(m_conceptOffset);
    if (conceptDescriptor == nullptr)
        return {};
    if (conceptDescriptor->actions() == nullptr)
        return {};
    if (conceptDescriptor->actions()->size() <= index)
        return {};
    return ConceptAction(m_reader, (void *) conceptDescriptor, index);
}

tu_uint8
lyric_object::ConceptWalker::numSealedSubConcepts() const
{
    if (!isValid())
        return 0;
    auto *conceptDescriptor = m_reader->getConcept(m_conceptOffset);
    if (conceptDescriptor == nullptr)
        return 0;
    if (conceptDescriptor->sealed_subtypes() == nullptr)
        return 0;
    return conceptDescriptor->sealed_subtypes()->size();
}

lyric_object::TypeWalker
lyric_object::ConceptWalker::getSealedSubConcept(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *conceptDescriptor = m_reader->getConcept(m_conceptOffset);
    if (conceptDescriptor == nullptr)
        return {};
    if (conceptDescriptor->sealed_subtypes() == nullptr)
        return {};
    if (conceptDescriptor->sealed_subtypes()->size() <= index)
        return {};
    return TypeWalker(m_reader, conceptDescriptor->sealed_subtypes()->Get(index));
}

lyric_object::TypeWalker
lyric_object::ConceptWalker::getConceptType() const
{
    if (!isValid())
        return {};
    auto *conceptDescriptor = m_reader->getConcept(m_conceptOffset);
    if (conceptDescriptor == nullptr)
        return {};
    if (conceptDescriptor->concept_type() == INVALID_ADDRESS_U32)
        return {};
    return TypeWalker(m_reader, conceptDescriptor->concept_type());
}

tu_uint32
lyric_object::ConceptWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_conceptOffset;
}
