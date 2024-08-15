
#include <lyric_common/common_conversions.h>
#include <tempo_config/config_result.h>

lyric_common::ModuleLocationParser::ModuleLocationParser()
{
}

lyric_common::ModuleLocationParser::ModuleLocationParser(
    const lyric_common::ModuleLocation &moduleLocationDefault)
    : m_default(moduleLocationDefault)
{
}

tempo_utils::Status
lyric_common::ModuleLocationParser::parseValue(
    const tempo_config::ConfigNode &node,
    ModuleLocation &moduleLocation) const
{
    if (node.isNil() && !m_default.isEmpty()) {
        moduleLocation = m_default.getValue();
        return tempo_config::ConfigStatus::ok();
    }
    if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType);

    auto value = node.toValue().getValue();
    auto moduleLocation__ = lyric_common::ModuleLocation::fromString(value);
    if (moduleLocation__.isValid()) {
        moduleLocation = std::move(moduleLocation__);
        return tempo_config::ConfigStatus::ok();
    }

    return tempo_config::ConfigStatus::forCondition(
        tempo_config::ConfigCondition::kParseError,
        "value '{}' cannot be converted to ModuleLocation", value);
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