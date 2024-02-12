#ifndef LYRIC_BUILD_BUILD_CONVERSIONS_H
#define LYRIC_BUILD_BUILD_CONVERSIONS_H

#include <lyric_build/build_types.h>
#include <tempo_config/abstract_config_parser.h>

namespace lyric_build {

    class TaskIdParser : public tempo_config::AbstractConfigParser<TaskId> {
    public:
        TaskIdParser();
        TaskIdParser(const TaskId &taskIdDefault);
        tempo_utils::Status parseValue(
            const tempo_config::ConfigNode &node,
            TaskId &taskId) const override;

    private:
        Option<TaskId> m_default;
    };
}

#endif // LYRIC_BUILD_BUILD_CONVERSIONS_H
