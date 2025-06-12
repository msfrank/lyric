#ifndef LYRIC_PARSER_INTERNAL_BASE_OPS_H
#define LYRIC_PARSER_INTERNAL_BASE_OPS_H

#include "module_archetype.h"

namespace lyric_parser::internal {

    class BaseOps {
    public:
        explicit BaseOps(ModuleArchetype *listener);

        ModuleArchetype *getListener() const;
        ArchetypeState *getState() const;

        bool hasError() const;
        void logErrorOrThrow( size_t lineNr, size_t columnNr, const std::string &message);
        void logErrorOrThrow(const antlr4::Token *token, const std::string &message);

        template<typename ...Args>
        void logErrorOrThrow(const antlr4::Token *token, fmt::string_view messageFmt, Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            logErrorOrThrow(token, message);
        }

    private:
        ModuleArchetype *m_listener;
    };
}
#endif // LYRIC_PARSER_INTERNAL_BASE_OPS_H
