
#include <absl/strings/str_split.h>

#include <lyric_build/build_conversions.h>
#include <lyric_build/build_types.h>
#include <tempo_config/config_result.h>

lyric_build::TaskIdParser::TaskIdParser()
{
}

lyric_build::TaskIdParser::TaskIdParser(const TaskId &taskIdDefault)
    : m_default(taskIdDefault)
{
}

tempo_utils::Status
lyric_build::TaskIdParser::convertValue(
    const tempo_config::ConfigNode &node,
    TaskId &taskId) const
{
    if (node.isNil() && !m_default.isEmpty()) {
        taskId = m_default.getValue();
        return {};
    }
    if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType);

    auto value = node.toValue().getValue();
    auto index = value.find(':');
    if (index == std::string::npos)
        return tempo_config::ConfigStatus::forCondition(
            tempo_config::ConfigCondition::kParseError,
            "value '{}' cannot be converted to TaskId", value);
    taskId = TaskId(value.substr(0, index), value.substr(index + 1));

    return {};
}