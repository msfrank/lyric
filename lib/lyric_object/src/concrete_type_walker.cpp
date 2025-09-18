
#include <lyric_object/concrete_type_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/internal/type_utils.h>
#include <lyric_object/type_walker.h>

lyric_object::ConcreteTypeWalker::ConcreteTypeWalker()
    : m_concreteAssignable(nullptr)
{
}

lyric_object::ConcreteTypeWalker::ConcreteTypeWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
     void *concreteAssignable)
    : m_reader(reader),
      m_concreteAssignable(concreteAssignable)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_concreteAssignable != nullptr);
}

lyric_object::ConcreteTypeWalker::ConcreteTypeWalker(const ConcreteTypeWalker &other)
    : m_reader(other.m_reader),
      m_concreteAssignable(other.m_concreteAssignable)
{
}

bool
lyric_object::ConcreteTypeWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_concreteAssignable;
}

lyric_common::TypeDef
lyric_object::ConcreteTypeWalker::getTypeDef() const
{
    if (!isValid())
        return {};
    auto *concreteAssignable = static_cast<const lyo1::ConcreteAssignable *>(m_concreteAssignable);

    auto loadTypeSymbolResult = internal::load_type_symbol(m_reader, {},
        concreteAssignable->concrete_section(), concreteAssignable->concrete_descriptor());
    if (loadTypeSymbolResult.isStatus())
        return {};
    auto concreteUrl = loadTypeSymbolResult.getResult();

    std::vector<lyric_common::TypeDef> parameters;
    if (concreteAssignable->concrete_parameters()) {
        for (const auto &index : *concreteAssignable->concrete_parameters()) {
            TypeWalker walker(m_reader, index);
            if (!walker.isValid())
                return {};
            auto parameter = walker.getTypeDef();
            if (!parameter.isValid())
                return {};
            parameters.push_back(parameter);
        }
    }

    auto result = lyric_common::TypeDef::forConcrete(concreteUrl, parameters);
    return result.orElse({});
}

lyric_object::LinkageSection
lyric_object::ConcreteTypeWalker::getLinkageSection() const
{
    if (!isValid())
        return {};
    auto *concreteAssignable = static_cast<const lyo1::ConcreteAssignable *>(m_concreteAssignable);
    switch (concreteAssignable->concrete_section()) {
        case lyo1::TypeSection::Binding:
            return LinkageSection::Binding;
        case lyo1::TypeSection::Call:
            return LinkageSection::Call;
        case lyo1::TypeSection::Class:
            return LinkageSection::Class;
        case lyo1::TypeSection::Concept:
            return LinkageSection::Concept;
        case lyo1::TypeSection::Enum:
            return LinkageSection::Enum;
        case lyo1::TypeSection::Existential:
            return LinkageSection::Existential;
        case lyo1::TypeSection::Instance:
            return LinkageSection::Instance;
        case lyo1::TypeSection::Static:
            return LinkageSection::Static;
        case lyo1::TypeSection::Struct:
            return LinkageSection::Struct;
        default:
            return LinkageSection::Invalid;
    }
}

tu_uint32
lyric_object::ConcreteTypeWalker::getLinkageIndex() const
{
    if (!isValid())
        return {};
    auto *concreteAssignable = static_cast<const lyo1::ConcreteAssignable *>(m_concreteAssignable);
    return concreteAssignable->concrete_descriptor();
}

std::vector<lyric_object::TypeWalker>
lyric_object::ConcreteTypeWalker::getParameters() const
{
    if (!isValid())
        return {};
    auto *concreteAssignable = static_cast<const lyo1::ConcreteAssignable *>(m_concreteAssignable);
    std::vector<TypeWalker> parameters;
    if (concreteAssignable->concrete_parameters()) {
        for (tu_uint32 p : *concreteAssignable->concrete_parameters()) {
            parameters.push_back(TypeWalker(m_reader, p));
        }
    }
    return parameters;
}
