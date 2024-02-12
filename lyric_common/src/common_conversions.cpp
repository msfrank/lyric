
#include <lyric_common/common_conversions.h>
#include <tempo_config/config_result.h>

lyric_common::AssemblyLocationParser::AssemblyLocationParser()
{
}

lyric_common::AssemblyLocationParser::AssemblyLocationParser(
    const lyric_common::AssemblyLocation &assemblyLocationDefault)
    : m_default(assemblyLocationDefault)
{
}

tempo_utils::Status
lyric_common::AssemblyLocationParser::parseValue(
    const tempo_config::ConfigNode &node,
    AssemblyLocation &assemblyLocation) const
{
    if (node.isNil() && !m_default.isEmpty()) {
        assemblyLocation = m_default.getValue();
        return tempo_config::ConfigStatus::ok();
    }
    if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType);

    auto value = node.toValue().getValue();
    auto assemblyLocation__ = lyric_common::AssemblyLocation::fromString(value);
    if (assemblyLocation__.isValid()) {
        assemblyLocation = std::move(assemblyLocation__);
        return tempo_config::ConfigStatus::ok();
    }

    return tempo_config::ConfigStatus::forCondition(
        tempo_config::ConfigCondition::kParseError,
        "value '{}' cannot be converted to AssemblyLocation", value);
}

lyric_common::SymbolPathParser::SymbolPathParser()
{
}

lyric_common::SymbolPathParser::SymbolPathParser(const SymbolPath &symbolPathDefault)
    : m_default(symbolPathDefault)
{
}

tempo_utils::Status
lyric_common::SymbolPathParser::parseValue(
    const tempo_config::ConfigNode &node,
    SymbolPath &symbolPath) const
{
    if (node.isNil() && !m_default.isEmpty()) {
        symbolPath = m_default.getValue();
        return tempo_config::ConfigStatus::ok();
    }
    if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType);

    auto value = node.toValue().getValue();
    auto symbolPath__ = SymbolPath::fromString(value);
    if (symbolPath__.isValid()) {
        symbolPath = std::move(symbolPath__);
        return tempo_config::ConfigStatus::ok();
    }

    return tempo_config::ConfigStatus::forCondition(
        tempo_config::ConfigCondition::kParseError,
        "value '{}' cannot be converted to SymbolPath", value);
}

lyric_common::SymbolUrlParser::SymbolUrlParser()
{
}

lyric_common::SymbolUrlParser::SymbolUrlParser(const SymbolUrl &symbolUrlDefault)
    : m_default(symbolUrlDefault)
{
}

tempo_utils::Status
lyric_common::SymbolUrlParser::parseValue(
    const tempo_config::ConfigNode &node,
    SymbolUrl &symbolUrl) const
{
    if (node.isNil() && !m_default.isEmpty()) {
        symbolUrl = m_default.getValue();
        return tempo_config::ConfigStatus::ok();
    }
    if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType);

    auto value = node.toValue().getValue();
    auto symbolUrl__ = SymbolUrl::fromString(value);
    if (symbolUrl__.isValid()) {
        symbolUrl = std::move(symbolUrl__);
        return tempo_config::ConfigStatus::ok();
    }

    return tempo_config::ConfigStatus::forCondition(
        tempo_config::ConfigCondition::kParseError,
        "value '{}' cannot be converted to SymbolUrl", value);
}