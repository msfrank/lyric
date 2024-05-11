#ifndef LYRIC_COMMON_RUNTIME_POLICY_H
#define LYRIC_COMMON_RUNTIME_POLICY_H

#include <vector>

#include <tempo_utils/url.h>

namespace lyric_common {

    enum class PolicyAction {
        ALLOW,
        DENY,
    };

    struct PolicyStatement {
        tempo_utils::Url subject;
        PolicyAction action;
        std::vector<tempo_utils::Url> resources;
    };

    struct RuntimePolicy {
        std::vector<PolicyStatement> statements;
    };
}

#endif // LYRIC_COMMON_RUNTIME_POLICY_H