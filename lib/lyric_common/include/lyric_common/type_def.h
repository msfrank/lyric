#ifndef LYRIC_COMMON_TYPE_DEF_H
#define LYRIC_COMMON_TYPE_DEF_H

#include <span>
#include <string>
#include <vector>

#include <lyric_common/symbol_url.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/result.h>

namespace lyric_common {

    enum class TypeDefType {
        Invalid,
        Concrete,
        Placeholder,
        Intersection,
        Union,
        NoReturn,
    };

    class TypeDef {
    public:
        TypeDef();
        TypeDef(const TypeDef &other);
        TypeDef(TypeDef &&other) noexcept;

        TypeDef &operator=(const TypeDef &other);
        TypeDef &operator=(TypeDef &&other) noexcept;

        bool isValid() const;

        TypeDefType getType() const;

        SymbolUrl getConcreteUrl() const;
        std::span<const TypeDef> getConcreteArguments() const;
        std::vector<TypeDef>::const_iterator concreteArgumentsBegin() const;
        std::vector<TypeDef>::const_iterator concreteArgumentsEnd() const;
        int numConcreteArguments() const;

        int getPlaceholderIndex() const;
        SymbolUrl getPlaceholderTemplateUrl() const;
        std::span<const TypeDef> getPlaceholderArguments() const;
        std::vector<TypeDef>::const_iterator placeholderArgumentsBegin() const;
        std::vector<TypeDef>::const_iterator placeholderArgumentsEnd() const;
        int numPlaceholderArguments() const;

        std::span<const TypeDef> getIntersectionMembers() const;
        std::vector<TypeDef>::const_iterator intersectionMembersBegin() const;
        std::vector<TypeDef>::const_iterator intersectionMembersEnd() const;
        int numIntersectionMembers() const;

        std::span<const TypeDef> getUnionMembers() const;
        std::vector<TypeDef>::const_iterator unionMembersBegin() const;
        std::vector<TypeDef>::const_iterator unionMembersEnd() const;
        int numUnionMembers() const;

        std::string toString() const;

        bool operator==(const TypeDef &other) const;
        bool operator!=(const TypeDef &other) const;

        static tempo_utils::Result<TypeDef> forConcrete(
            const SymbolUrl &concreteUrl,
            const std::vector<TypeDef> &concreteArguments = {});
        static tempo_utils::Result<TypeDef> forPlaceholder(
            int placeholderIndex,
            const SymbolUrl &placeholderTemplateUrl,
            const std::vector<TypeDef> &placeholderArguments = {});
        static tempo_utils::Result<TypeDef> forIntersection(const std::vector<TypeDef> &intersectionMembers);
        static tempo_utils::Result<TypeDef> forUnion(const std::vector<TypeDef> &unionMembers);

        static TypeDef noReturn();

    private:
        struct Priv {
            TypeDefType type;
            SymbolUrl symbol;
            tempo_utils::PrehashedValue<std::vector<TypeDef>> parameters;
            int placeholder;
            Priv(
                TypeDefType type_,
                const lyric_common::SymbolUrl &symbol_,
                const std::vector<TypeDef> &parameters_,
                int placeholder_)
                : type(type_),
                  symbol(symbol_),
                  parameters(tempo_utils::make_prehashed<std::vector<TypeDef>>(parameters_.cbegin(), parameters_.cend())),
                  placeholder(placeholder_) {};
        };
        std::shared_ptr<const Priv> m_priv;

        TypeDef(
            TypeDefType type,
            const lyric_common::SymbolUrl &symbol,
            const std::vector<TypeDef> &parameters,
            int placeholder);

        friend bool member_cmp(const lyric_common::TypeDef &lhs, const lyric_common::TypeDef &rhs);

    public:
        template <typename H>
        friend H AbslHashValue(H h, const TypeDef &typeDef) {
            auto &priv = *typeDef.m_priv;
            switch (typeDef.m_priv->type) {
                case TypeDefType::Concrete:
                    return H::combine(std::move(h), priv.type, priv.symbol, priv.parameters);
                case TypeDefType::Placeholder:
                    return H::combine(std::move(h), priv.type, priv.placeholder, priv.symbol, priv.parameters);
                case TypeDefType::Intersection:
                case TypeDefType::Union:
                    return H::combine(std::move(h), priv.type, priv.parameters);
                case TypeDefType::NoReturn:
                case TypeDefType::Invalid:
                    return H::combine(std::move(h), priv.type);
            }
            TU_UNREACHABLE();
        }
    };

    tempo_utils::LogMessage &&operator<<(tempo_utils::LogMessage &&message, const TypeDef &type);
}

#endif // LYRIC_COMMON_TYPE_DEF_H
