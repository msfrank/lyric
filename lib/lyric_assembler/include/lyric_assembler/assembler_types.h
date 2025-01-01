#ifndef LYRIC_ASSEMBLER_ASSEMBLER_TYPES_H
#define LYRIC_ASSEMBLER_ASSEMBLER_TYPES_H

#include <absl/container/flat_hash_map.h>

#include <lyric_common/module_location.h>
#include <lyric_common/symbol_path.h>
#include <lyric_common/symbol_url.h>
#include <lyric_runtime/literal_cell.h>
#include <lyric_runtime/runtime_types.h>
#include <tempo_utils/option_template.h>

namespace lyric_assembler {

    /** */
    constexpr int kNumFunctionClasses = 8;

    /** */
    constexpr int kNumTupleClasses = 8;

    enum class FundamentalSymbol {
        Any,
        Arithmetic,
        Binding,
        Bool,
        Bytes,
        Call,
        Category,
        Char,
        Class,
        Comparison,
        Concept,
        Descriptor,
        Enum,
        Equality,
        Float,
        Idea,
        Instance,
        Int,
        Intrinsic,
        Iterable,
        Iterator,
        Map,
        Namespace,
        Nil,
        Object,
        Ordered,
        Pair,
        Proposition,
        Record,
        Seq,
        Singleton,
        Status,
        String,
        Struct,
        Type,
        Undef,
        Unwrap,
        Url,

        // statuses
        Cancelled,
        InvalidArgument,
        DeadlineExceeded,
        NotFound,
        AlreadyExists,
        PermissionDenied,
        Unauthenticated,
        ResourceExhausted,
        FailedPrecondition,
        Aborted,
        Unavailable,
        OutOfRange,
        Unimplemented,
        Internal,
        DataLoss,
        Unknown,

        // functions
        Function0,
        Function1,
        Function2,
        Function3,
        Function4,
        Function5,
        Function6,
        Function7,

        // tuples
        Tuple1,
        Tuple2,
        Tuple3,
        Tuple4,
        Tuple5,
        Tuple6,
        Tuple7,

        // companion instances
        BoolInstance,
        CharInstance,
        FloatInstance,
        IntInstance,
        BytesInstance,
        StringInstance,
        UrlInstance,

        // tuple instances
        Tuple1Instance,
        Tuple2Instance,
        Tuple3Instance,
        Tuple4Instance,
        Tuple5Instance,
        Tuple6Instance,
        Tuple7Instance,

        // sentinel, must be last
        NUM_SYMBOLS,
    };

    std::string fundamentalTypeToString(FundamentalSymbol fundamentalType);
    lyric_common::SymbolPath fundamentalTypeToSymbolPath(FundamentalSymbol fundamentalType);

    enum class BindingType {
        Invalid,
        Descriptor,                     /**< binding refers to a symbol descriptor */
        Placeholder,                    /**< binding refers to a template placeholder */
        Value,                          /**< binding refers to a data value */
        Variable,                       /**< binding refers to a data variable */
        Namespace,                      /**< binding refers to a namespace */
    };

    enum class ReferenceType {
        Invalid,
        Descriptor,                     /**< binding refers to a symbol descriptor */
        Value,                          /**< binding refers to a data value */
        Variable,                       /**< binding refers to a data variable */
        Namespace,                      /**< binding refers to a namespace */
    };

    enum class DeriveType {
        Any,
        Sealed,
        Final,
    };

    enum class SyntheticType {
        This,
    };

    enum class ImportFlags {
        SystemBootstrap,
        ExactLinkage,
        ApiLinkage,
    };

    struct ImportHandle {
        lyric_common::ModuleLocation location;
        ImportFlags flags;
        bool isShared;
    };

    struct RequestedLink {
        lyric_object::LinkageSection linkType;
        lyric_common::SymbolUrl linkUrl;
    };

    struct JumpTarget {
        JumpTarget() : m_targetId(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit JumpTarget(tu_uint32 targetId) : m_targetId(targetId) { TU_ASSERT(m_targetId != lyric_runtime::INVALID_ADDRESS_U32); };
        JumpTarget(const JumpTarget &other) : m_targetId(other.m_targetId) {};
        bool isValid() const { return m_targetId != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getId() const { return m_targetId; };
    private:
        tu_uint32 m_targetId;
    };

    struct JumpLabel {
        JumpLabel() {};
        explicit JumpLabel(std::string_view label) : m_label(label) { TU_ASSERT(!m_label.empty()); };
        JumpLabel(const JumpLabel &other) : m_label(other.m_label) {};
        bool isValid() const { return !m_label.empty(); };
        std::string getLabel() const { return m_label; };
        std::string_view labelView() const { return m_label; };
    private:
        std::string m_label;
    };

    struct ArgumentOffset {
    public:
        ArgumentOffset() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit ArgumentOffset(tu_uint32 u32) : u32(u32) {};
        ArgumentOffset(const ArgumentOffset &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getOffset() const { return u32; };
        bool operator==(const ArgumentOffset &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = lyric_runtime::INVALID_ADDRESS_U32;
    };

    struct LocalOffset {
    public:
        LocalOffset() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit LocalOffset(tu_uint32 u32) : u32(u32) {};
        LocalOffset(const LocalOffset &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getOffset() const { return u32; };
        bool operator==(const LocalOffset &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = lyric_runtime::INVALID_ADDRESS_U32;
    };

    struct LexicalOffset {
    public:
        LexicalOffset() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit LexicalOffset(tu_uint32 u32) : u32(u32) {};
        LexicalOffset(const LexicalOffset &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getOffset() const { return u32; };
        bool operator==(const LexicalOffset &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = lyric_runtime::INVALID_ADDRESS_U32;
    };

    struct RestOffset {
    public:
        RestOffset() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit RestOffset(tu_uint32 u32) : u32(u32) {};
        RestOffset(const RestOffset &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getOffset() const { return u32; };
        bool operator==(const RestOffset &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = lyric_runtime::INVALID_ADDRESS_U32;
    };

    // forward declarations
    class TypeHandle;

    class TypeSignature {

    public:
        TypeSignature();
        explicit TypeSignature(const std::vector<const TypeHandle *> &signature);
        TypeSignature(const TypeSignature &other);

        bool isValid() const;

        lyric_runtime::TypeComparison compare(const TypeSignature &other) const;
        std::vector<const TypeHandle *> getSignature() const;
        std::vector<const TypeHandle *>::const_iterator signatureBegin() const;
        std::vector<const TypeHandle *>::const_iterator signatureEnd() const;

        bool operator==(const TypeSignature &other) const;
        bool operator!=(const TypeSignature &other) const;

    private:
        std::vector<const TypeHandle *> m_signature;
    };

    /**
     * A binding of a symbol to a block. A symbol can be bound to multiple blocks, and a symbol can be
     * bound to the same block multiple times as long as each binding has a different name.
     */
    struct SymbolBinding {
        BindingType bindingType;
        lyric_common::SymbolUrl symbolUrl;
        lyric_common::TypeDef typeDef;
        SymbolBinding();
        SymbolBinding(
            BindingType bindingType,
            const lyric_common::SymbolUrl &symbol,
            const lyric_common::TypeDef &type);
    };

    /**
     * A reference to a datum. The datum may exist on the current stack frame (an argument, local, or
     * rest cell), a parent frame (a lexical cell), or in a segment (a static cell).
     */
    struct DataReference {
        ReferenceType referenceType;                /**< the reference type */
        lyric_common::SymbolUrl symbolUrl;          /**< The symbol which defines the datum */
        lyric_common::TypeDef typeDef;              /**< The type of the datum */
        DataReference();
        DataReference(
            ReferenceType referenceType,
            const lyric_common::SymbolUrl &symbolUrl,
            const lyric_common::TypeDef &typeDef);
    };

    struct Parameter {
        tu_uint8 index;
        std::string name;                           // name of the parameter
        std::string label;                          // optional parameter label
        lyric_common::TypeDef typeDef;              // type of the parameter
        lyric_object::PlacementType placement;
        bool isVariable;
    };

    struct ParameterPack {
        std::vector<Parameter> listParameters;
        std::vector<Parameter> namedParameters;
        Option<Parameter> restParameter;
    };

    struct ActionMethod {
        lyric_common::SymbolUrl methodAction;
        ActionMethod();
        ActionMethod(const lyric_common::SymbolUrl &methodAction);
    };

    struct BoundMethod {
        lyric_common::SymbolUrl methodCall;
        lyric_object::AccessType access;
        bool final;
        BoundMethod();
        BoundMethod(
            const lyric_common::SymbolUrl &methodCall,
            lyric_object::AccessType access,
            bool final);
    };

    struct ExtensionMethod {
        lyric_common::SymbolUrl methodCall;
        lyric_common::SymbolUrl methodAction;
        ExtensionMethod();
        ExtensionMethod(
            const lyric_common::SymbolUrl &methodCall,
            const lyric_common::SymbolUrl &methodAction);
    };

    class CondWhenPatch {
    public:
        CondWhenPatch();
        CondWhenPatch(
            const JumpLabel &predicateLabel,
            const JumpTarget &predicateJump,
            const JumpTarget &consequentJump);
        CondWhenPatch(const CondWhenPatch &other);

        JumpLabel getPredicateLabel() const;
        JumpTarget getPredicateJump() const;
        JumpTarget getConsequentJump() const;

    private:
        JumpLabel m_predicateLabel;
        JumpTarget m_predicateJump;
        JumpTarget m_consequentJump;
    };

    class MatchWhenPatch {
    public:
        MatchWhenPatch();
        MatchWhenPatch(
            const lyric_common::TypeDef &predicateType,
            const JumpLabel &predicateLabel,
            const JumpTarget &predicateJump,
            const JumpTarget &consequentJump,
            const lyric_common::TypeDef &consequentType);
        MatchWhenPatch(const MatchWhenPatch &other);

        lyric_common::TypeDef getPredicateType() const;
        JumpLabel getPredicateLabel() const;
        JumpTarget getPredicateJump() const;
        JumpTarget getConsequentJump() const;
        lyric_common::TypeDef getConsequentType() const;

    private:
        lyric_common::TypeDef m_predicateType;
        JumpLabel m_predicateLabel;
        JumpTarget m_predicateJump;
        JumpTarget m_consequentJump;
        lyric_common::TypeDef m_consequentType;

    };

    /**
     * Describes an import reference, which contains a symbol path and optional symbol alias.
     */
    class ImportRef {
    public:
        ImportRef();
        ImportRef(const lyric_common::SymbolPath &path);
        ImportRef(const lyric_common::SymbolPath &path, std::string_view alias);
        ImportRef(const ImportRef &other);

        bool isValid() const;
        lyric_common::SymbolPath getPath() const;
        std::string getName() const;

        bool operator==(const ImportRef &other) const;
        bool operator!=(const ImportRef &other) const;

        /**
         * The hash function hashes the symbol path only.
         */
        template<typename H>
        friend H AbslHashValue(H h, const ImportRef &importRef)
        {
            return H::combine(std::move(h), importRef.m_path);
        }

    private:
        lyric_common::SymbolPath m_path;
        std::string m_name;
    };

    /**
     *
     */
    struct ImplRef {
        lyric_common::SymbolUrl receiverUrl;
        lyric_common::TypeDef implType;

        bool operator==(const ImplRef &other) const {
            return receiverUrl == other.receiverUrl && implType == other.implType;
        }

        template <typename H>
        friend H AbslHashValue(H h, const ImplRef &implRef) {
            return H::combine(std::move(h), implRef.receiverUrl, implRef.implType);
        }
    };
}

#endif // LYRIC_ASSEMBLER_ASSEMBLER_TYPES_H