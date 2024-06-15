
#include <lyric_assembler/assembler_result.h>
#include <lyric_assembler/pack_builder.h>

lyric_assembler::PackBuilder::PackBuilder()
    : m_currindex(0),
      m_firstlistopt(-1)
{
}

tempo_utils::Status
lyric_assembler::PackBuilder::check(std::string_view name, std::string_view label)
{
    if (m_listParameters.size() + m_namedParameters.size() == std::numeric_limits<tu_uint8>::max())
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "cannot define more than {} parameters", std::numeric_limits<tu_uint8>::max());
    if (m_usedNames.contains(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "invalid parameter at index {}; parameter name {} is already defined", m_currindex, name);
    if (m_usedLabels.contains(label))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "invalid parameter at index {}; parameter label {} is already defined", m_currindex, label);
    return {};
}

tempo_utils::Status
lyric_assembler::PackBuilder::appendListParameter(
    std::string_view name,
    std::string_view label,
    const lyric_common::TypeDef &type,
    bool isVariable)
{
    TU_RETURN_IF_NOT_OK (check(name, label));
    if (m_firstlistopt >= 0)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "invalid parameter at index {}; all list parameters after index {} must be default-initialized",
            m_currindex, m_firstlistopt);
    Parameter p;
    p.index = m_currindex++;
    p.name = name;
    p.label = label;
    p.typeDef = type;
    p.placement = lyric_object::PlacementType::List;
    p.isVariable = isVariable;
    m_listParameters.push_back(p);
    return {};
}

tempo_utils::Status
lyric_assembler::PackBuilder::appendListOptParameter(
    std::string_view name,
    std::string_view label,
    const lyric_common::TypeDef &type,
    bool isVariable)
{
    TU_RETURN_IF_NOT_OK (check(name, label));
    if (m_firstlistopt < 0) {
        m_firstlistopt = m_currindex;
    }
    Parameter p;
    p.index = m_currindex++;
    p.name = name;
    p.label = label;
    p.typeDef = type;
    p.placement = lyric_object::PlacementType::ListOpt;
    p.isVariable = isVariable;
    m_listParameters.push_back(p);
    return {};
}

tempo_utils::Status
lyric_assembler::PackBuilder::appendNamedParameter(
    std::string_view name,
    std::string_view label,
    const lyric_common::TypeDef &type,
    bool isVariable)
{
    TU_RETURN_IF_NOT_OK (check(name, label));
    Parameter p;
    p.index = m_currindex++;
    p.name = name;
    p.label = label;
    p.typeDef = type;
    p.placement = lyric_object::PlacementType::Named;
    p.isVariable = isVariable;
    m_namedParameters.push_back(p);
    return {};
}

tempo_utils::Status
lyric_assembler::PackBuilder::appendNamedOptParameter(
    std::string_view name,
    std::string_view label,
    const lyric_common::TypeDef &type,
    bool isVariable)
{
    TU_RETURN_IF_NOT_OK (check(name, label));
    Parameter p;
    p.index = m_currindex++;
    p.name = name;
    p.label = label;
    p.typeDef = type;
    p.placement = lyric_object::PlacementType::NamedOpt;
    p.isVariable = isVariable;
    m_namedParameters.push_back(p);
    return {};
}

tempo_utils::Status
lyric_assembler::PackBuilder::appendCtxParameter(
    std::string_view name,
    std::string_view label,
    const lyric_common::TypeDef &type)
{
    TU_RETURN_IF_NOT_OK (check(name, label));
    Parameter p;
    p.index = m_currindex++;
    p.name = name;
    p.label = label;
    p.typeDef = type;
    p.placement = lyric_object::PlacementType::Ctx;
    p.isVariable = false;
    m_namedParameters.push_back(p);
    return {};
}

tempo_utils::Status
lyric_assembler::PackBuilder::appendRestParameter(
    std::string_view name,
    const lyric_common::TypeDef &type)
{
    if (!m_restParameter.isEmpty())
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "rest parameter is already defined");
    Parameter p;
    p.index = -1;
    p.name = name;
    p.label = name;
    p.typeDef = type;
    p.placement = lyric_object::PlacementType::Rest;
    p.isVariable = false;
    m_restParameter = Option(p);
    return {};
}

tempo_utils::Result<lyric_assembler::ParameterPack>
lyric_assembler::PackBuilder::toParameterPack() const
{
    return ParameterPack{m_listParameters, m_namedParameters, m_restParameter};
}
