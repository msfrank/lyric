#ifndef LYRIC_TYPING_TYPE_SPEC_H
#define LYRIC_TYPING_TYPE_SPEC_H

#include <absl/hash/hash.h>

#include <lyric_common/type_def.h>
#include <lyric_common/symbol_url.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/url.h>

namespace lyric_typing {

    enum class TypeSpecType {
        Invalid,
        Singular,
        Intersection,
        Union,
    };

    class TypeSpec {

    public:
        TypeSpec();
        TypeSpec(const TypeSpec &other);

        bool isValid() const;

        TypeSpecType getType() const;
        lyric_common::ModuleLocation getTypeLocation() const;
        lyric_common::SymbolPath getTypePath() const;
        std::vector<TypeSpec> getTypeParameters() const;
        std::vector<TypeSpec> getIntersection() const;
        std::vector<TypeSpec> getUnion() const;

        std::string toString() const;

        bool operator==(const TypeSpec &other) const;
        bool operator!=(const TypeSpec &other) const;

        static TypeSpec forSingular(
            const lyric_common::SymbolPath &symbolPath,
            const std::vector<TypeSpec> &parameters = {});
        static TypeSpec forSingular(
            std::initializer_list<std::string> symbolPath,
            const std::vector<TypeSpec> &parameters = {});
        static TypeSpec forSingular(
            const lyric_common::SymbolUrl &symbolUrl,
            const std::vector<TypeSpec> &parameters = {});
        static TypeSpec forSingular(
            const tempo_utils::Url &location,
            const lyric_common::SymbolPath &path,
            const std::vector<TypeSpec> &parameters = {});
        static TypeSpec forSingular(
            const tempo_utils::Url &location,
            std::initializer_list<std::string> symbolPath,
            const std::vector<TypeSpec> &parameters = {});
        static TypeSpec forIntersection(const std::vector<TypeSpec> &members);
        static TypeSpec forUnion(const std::vector<TypeSpec> &members);

        static TypeSpec fromTypeDef(const lyric_common::TypeDef &typeDef);

        template<typename H>
        friend H AbslHashValue(H h, const TypeSpec &assignable) {
            switch (assignable.getType()) {
                case TypeSpecType::Singular:
                    return H::combine(std::move(h), assignable.m_symbolUrl, assignable.m_parameters);
                case TypeSpecType::Intersection:
                    return H::combine(std::move(h), assignable.m_parameters);
                case TypeSpecType::Union:
                    return H::combine(std::move(h), assignable.m_parameters);
                case TypeSpecType::Invalid:
                    return H::combine(std::move(h), assignable.m_type);
            }
        }

    private:
        TypeSpecType m_type;
        lyric_common::SymbolUrl m_symbolUrl;
        std::vector<TypeSpec> m_parameters;

        TypeSpec(
            TypeSpecType type,
            const lyric_common::SymbolUrl &symbolUrl,
            const std::vector<TypeSpec> &parameters);
    };

    tempo_utils::LogMessage &&operator<<(tempo_utils::LogMessage &&message, const TypeSpec &assignable);

}

#endif // LYRIC_TYPING_TYPE_SPEC_H
