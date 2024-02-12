//#ifndef LYRIC_ASSEMBLER_ASSIGNABLE_H
//#define LYRIC_ASSEMBLER_ASSIGNABLE_H
//
//#include <string>
//#include <vector>
//
//#include <absl/hash/hash.h>
//
//#include <lyric_common/symbol_url.h>
//#include <lyric_parser/assignable.h>
//#include <tempo_utils/log_message.h>
//
//namespace lyric_assembler {
//
//    enum class AssignableType {
//        INVALID,
//        CONCRETE,
//        PLACEHOLDER,
//        INTERSECTION,
//        UNION,
//    };
//
//    class Assignable {
//    public:
//        Assignable();
//        Assignable(const Assignable &other);
//        Assignable(Assignable &&other) noexcept;
//        ~Assignable();
//
//        Assignable &operator=(const Assignable &other);
//        Assignable &operator=(Assignable &&other) noexcept;
//
//        bool isValid() const;
//
//        AssignableType getType() const;
//        lyric_common::SymbolUrl getConcreteUrl() const;
//        int getPlaceholderIndex() const;
//        lyric_common::SymbolUrl getTemplateUrl() const;
//        std::vector<Assignable> getTypeParameters() const;
//        std::vector<Assignable> getIntersection() const;
//        std::vector<Assignable> getUnion() const;
//
//        lyric_parser::Assignable toSpec() const;
//        std::string toString() const;
//
//        bool operator==(const Assignable &other) const;
//        bool operator!=(const Assignable &other) const;
//
//        static Assignable forConcrete(
//            const lyric_common::SymbolUrl &concreteUrl,
//            const std::vector<Assignable> &typeParameters = {});
//        static Assignable forPlaceholder(
//            int placeholderIndex,
//            const lyric_common::SymbolUrl &templateUrl,
//            const std::vector<Assignable> &typeParameters = {});
//        static Assignable forIntersection(const std::vector<Assignable> &members);
//        static Assignable forUnion(const std::vector<Assignable> &members);
//
//        template<typename H>
//        friend H AbslHashValue(H h, const Assignable &assignable) {
//            switch (assignable.m_priv->type) {
//                case AssignableType::CONCRETE:
//                    return H::combine(std::move(h),
//                                      assignable.m_priv->type,
//                                      assignable.m_priv->symbol,
//                                      assignable.m_priv->parameters);
//                case AssignableType::PLACEHOLDER:
//                    return H::combine(std::move(h),
//                                      assignable.m_priv->type,
//                                      assignable.m_priv->placeholder,
//                                      assignable.m_priv->symbol,
//                                      assignable.m_priv->parameters);
//                case AssignableType::INTERSECTION:
//                    return H::combine(std::move(h), assignable.m_priv->type, assignable.m_priv->parameters);
//                case AssignableType::UNION:
//                    return H::combine(std::move(h), assignable.m_priv->type, assignable.m_priv->parameters);
//                case AssignableType::INVALID:
//                    return H::combine(std::move(h), assignable.m_priv->type);
//            }
//        }
//
//        friend class AssemblyState;
//
//    private:
//        struct AssignablePriv {
//            AssignableType type;
//            lyric_common::SymbolUrl symbol;
//            std::vector<Assignable> parameters;
//            int placeholder;
//        };
//        std::shared_ptr<const AssignablePriv> m_priv;
//
//        Assignable(
//            AssignableType type,
//            const lyric_common::SymbolUrl &symbol,
//            const std::vector<Assignable> &parameters,
//            int placeholder);
//    };
//
//    tempo_utils::LogMessage &&operator<<(tempo_utils::LogMessage &&message, const Assignable &type);
//}
//
//#endif // LYRIC_ASSEMBLER_ASSIGNABLE_H
