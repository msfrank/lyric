
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
lyric_common::ModuleLocationParser::convertValue(
    const tempo_config::ConfigNode &node,
    ModuleLocation &moduleLocation) const
{
    if (node.isNil() && !m_default.isEmpty()) {
        moduleLocation = m_default.getValue();
        return {};
    }
    if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType);

    auto value = node.toValue().getValue();
    auto convertedLocation = lyric_common::ModuleLocation::fromString(value);
    if (convertedLocation.isValid()) {
        moduleLocation = std::move(convertedLocation);
        return {};
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
lyric_common::SymbolPathParser::convertValue(
    const tempo_config::ConfigNode &node,
    SymbolPath &symbolPath) const
{
    if (node.isNil() && !m_default.isEmpty()) {
        symbolPath = m_default.getValue();
        return {};
    }
    if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType);

    auto value = node.toValue().getValue();
    auto convertedPath = SymbolPath::fromString(value);
    if (convertedPath.isValid()) {
        symbolPath = std::move(convertedPath);
        return {};
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
lyric_common::SymbolUrlParser::convertValue(
    const tempo_config::ConfigNode &node,
    SymbolUrl &symbolUrl) const
{
    if (node.isNil() && !m_default.isEmpty()) {
        symbolUrl = m_default.getValue();
        return {};
    }
    if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType);

    auto value = node.toValue().getValue();
    auto convertedUrl = SymbolUrl::fromString(value);
    if (convertedUrl.isValid()) {
        symbolUrl = std::move(convertedUrl);
        return {};
    }

    return tempo_config::ConfigStatus::forCondition(
        tempo_config::ConfigCondition::kParseError,
        "value '{}' cannot be converted to SymbolUrl", value);
}