#ifndef LYRIC_PARSER_ASSIGNABLE_H
#define LYRIC_PARSER_ASSIGNABLE_H

#include <absl/hash/hash.h>

#include <lyric_common/type_def.h>
#include <lyric_common/symbol_url.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/url.h>

#include "parser_types.h"

namespace lyric_parser {

    enum class AssignableType {
        INVALID,
        SINGULAR,
        INTERSECTION,
        UNION,
    };

    class Assignable {

    public:
        Assignable();
        Assignable(const Assignable &other);

        bool isValid() const;

        AssignableType getType() const;
        lyric_common::AssemblyLocation getTypeLocation() const;
        lyric_common::SymbolPath getTypePath() const;
        std::vector<Assignable> getTypeParameters() const;
        std::vector<Assignable> getIntersection() const;
        std::vector<Assignable> getUnion() const;

        std::string toString() const;

        bool operator==(const Assignable &other) const;
        bool operator!=(const Assignable &other) const;

        static Assignable forSingular(
            const lyric_common::SymbolPath &symbolPath,
            const std::vector<Assignable> &parameters = {});
        static Assignable forSingular(
            std::initializer_list<std::string> symbolPath,
            const std::vector<Assignable> &parameters = {});
        static Assignable forSingular(
            const lyric_common::SymbolUrl &symbolUrl,
            const std::vector<Assignable> &parameters = {});
        static Assignable forSingular(
            const tempo_utils::Url &location,
            const lyric_common::SymbolPath &path,
            const std::vector<Assignable> &parameters = {});
        static Assignable forSingular(
            const tempo_utils::Url &location,
            std::initializer_list<std::string> symbolPath,
            const std::vector<Assignable> &parameters = {});
        static Assignable forIntersection(const std::vector<Assignable> &members);
        static Assignable forUnion(const std::vector<Assignable> &members);

        static Assignable fromTypeDef(const lyric_common::TypeDef &typeDef);

        template<typename H>
        friend H AbslHashValue(H h, const Assignable &assignable) {
            switch (assignable.getType()) {
                case AssignableType::SINGULAR:
                    return H::combine(std::move(h), assignable.m_symbolUrl, assignable.m_parameters);
                case AssignableType::INTERSECTION:
                    return H::combine(std::move(h), assignable.m_parameters);
                case AssignableType::UNION:
                    return H::combine(std::move(h), assignable.m_parameters);
                case AssignableType::INVALID:
                    return H::combine(std::move(h), assignable.m_type);
            }
        }

    private:
        AssignableType m_type;
        lyric_common::SymbolUrl m_symbolUrl;
        std::vector<Assignable> m_parameters;

        Assignable(
            AssignableType type,
            const lyric_common::SymbolUrl &symbolUrl,
            const std::vector<Assignable> &parameters);
    };

    tempo_utils::LogMessage &&operator<<(tempo_utils::LogMessage &&message, const Assignable &assignable);

}

#endif // LYRIC_PARSER_ASSIGNABLE_H
