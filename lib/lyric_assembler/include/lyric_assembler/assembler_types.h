#ifndef LYRIC_ASSEMBLER_ASSEMBLER_TYPES_H
#define LYRIC_ASSEMBLER_ASSEMBLER_TYPES_H

#include <absl/container/flat_hash_map.h>

#include <lyric_common/assembly_location.h>
#include <lyric_common/symbol_path.h>
#include <lyric_common/symbol_url.h>
#include <lyric_parser/assignable.h>
#include <lyric_parser/node_walker.h>
#include <lyric_parser/parser_types.h>
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
        Bool,
        Call,
        Category,
        Char,
        Class,
        Comparison,
        Concept,
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
    };

    enum class ReferenceType {
        Invalid,
        Descriptor,                     /**< binding refers to a symbol descriptor */
        Value,                          /**< binding refers to a data value */
        Variable,                       /**< binding refers to a data variable */
    };

    enum class PlacementType {
        INVALID,
        CTX,
        NAMED,
        OPT,
        LIST,
        REST,
    };

    enum class DeriveType {
        ANY,
        SEALED,
        FINAL,
    };

    enum class CallMode {
        NORMAL,
        CTOR,
        INLINE,
    };

    enum class SyntheticType {
        THIS,
    };

    enum class ImportFlags {
        SystemBootstrap,
        ExactLinkage,
        ApiLinkage,
    };

    struct ImportHandle {
        lyric_common::AssemblyLocation location;
        tu_uint32 importIndex;
        ImportFlags flags;
        bool isShared;
    };

    struct RequestedLink {
        lyric_object::LinkageSection linkType;
        lyric_common::SymbolUrl linkUrl;
    };

    struct PatchOffset {
        PatchOffset() : u16(lyric_runtime::INVALID_OFFSET_U16) {};
        explicit PatchOffset(tu_uint16 offset) : u16(offset) { TU_ASSERT(offset >> 15 == 0); };
        PatchOffset(const PatchOffset &other) : u16(other.u16) {};
        bool isValid() const { return u16 != lyric_runtime::INVALID_OFFSET_U16; }
        tu_uint16 getOffset() const { return u16; };
    private:
        tu_uint16 u16;
    };

    struct JumpLabel {
        JumpLabel() : u16(lyric_runtime::INVALID_OFFSET_U16) {};
        explicit JumpLabel(tu_uint16 offset) : u16(offset) { TU_ASSERT(offset >> 15 == 0); };
        JumpLabel(const JumpLabel &other) : u16(other.u16) {};
        bool isValid() const { return u16 != lyric_runtime::INVALID_OFFSET_U16; }
        tu_uint16 getOffset() const { return u16; };
    private:
        tu_uint16 u16;
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

    struct ImplOffset {
    public:
        ImplOffset() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit ImplOffset(tu_uint32 u32) : u32(u32) {};
        ImplOffset(const ImplOffset &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getOffset() const { return u32; };
        bool operator==(const ImplOffset &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = lyric_runtime::INVALID_ADDRESS_U32;
    };

    struct LiteralAddress {
        tu_uint32 addr;
        LiteralAddress() = default;
        explicit LiteralAddress(tu_uint32 addr_) : addr(addr_) { TU_ASSERT(addr >> 31 == 0); };
    };

    class NamespaceAddress {
    public:
        NamespaceAddress() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit NamespaceAddress(tu_uint32 u32) : u32(u32) {};
        NamespaceAddress(const NamespaceAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const NamespaceAddress &other) const { return u32 == other.u32; };
        bool operator!=(const NamespaceAddress &other) const { return u32 != other.u32; };
        static NamespaceAddress near(int index) { return NamespaceAddress(static_cast<tu_uint32>(std::abs(index))); };
        static NamespaceAddress far(int index) { return NamespaceAddress(static_cast<tu_uint32>(std::abs(index)) | 0x80000000); };
    private:
        tu_uint32 u32;
    };

    class ExistentialAddress {
    public:
        ExistentialAddress() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit ExistentialAddress(tu_uint32 u32) : u32(u32) {};
        ExistentialAddress(const ExistentialAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const ExistentialAddress &other) const { return u32 == other.u32; };
        bool operator!=(const ExistentialAddress &other) const { return u32 != other.u32; };
        static ExistentialAddress near(int index) { return ExistentialAddress(static_cast<tu_uint32>(std::abs(index))); };
        static ExistentialAddress far(int index) { return ExistentialAddress(static_cast<tu_uint32>(std::abs(index)) | 0x80000000); };
    private:
        tu_uint32 u32;
    };

    class CallAddress {
    public:
        CallAddress() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit CallAddress(tu_uint32 u32) : u32(u32) {};
        CallAddress(const CallAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const CallAddress &other) const { return u32 == other.u32; };
        bool operator!=(const CallAddress &other) const { return u32 != other.u32; };
        static CallAddress near(int index) { return CallAddress(static_cast<tu_uint32>(std::abs(index))); };
        static CallAddress far(int index) { return CallAddress(static_cast<tu_uint32>(std::abs(index)) | 0x80000000); };
    private:
        tu_uint32 u32;
    };

    class FieldAddress {
    public:
        FieldAddress() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit FieldAddress(tu_uint32 u32) : u32(u32) {};
        FieldAddress(const FieldAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const FieldAddress &other) const { return u32 == other.u32; };
        bool operator!=(const FieldAddress &other) const { return u32 != other.u32; };
        static FieldAddress near(int index) { return FieldAddress(static_cast<tu_uint32>(std::abs(index))); };
        static FieldAddress far(int index) { return FieldAddress(static_cast<tu_uint32>(std::abs(index)) | 0x80000000); };
    private:
        tu_uint32 u32;
    };

    class ActionAddress {
    public:
        ActionAddress() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit ActionAddress(tu_uint32 u32) : u32(u32) {};
        ActionAddress(const ActionAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const ActionAddress &other) const { return u32 == other.u32; };
        bool operator!=(const ActionAddress &other) const { return u32 != other.u32; };
        static ActionAddress near(int index) { return ActionAddress(static_cast<tu_uint32>(std::abs(index))); };
        static ActionAddress far(int index) { return ActionAddress(static_cast<tu_uint32>(std::abs(index)) | 0x80000000); };
    private:
        tu_uint32 u32;
    };

    class ConceptAddress {
    public:
        ConceptAddress() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit ConceptAddress(tu_uint32 u32) : u32(u32) {};
        ConceptAddress(const ConceptAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const ConceptAddress &other) const { return u32 == other.u32; };
        bool operator!=(const ConceptAddress &other) const { return u32 != other.u32; };
        static ConceptAddress near(int index) { return ConceptAddress(static_cast<tu_uint32>(std::abs(index))); };
        static ConceptAddress far(int index) { return ConceptAddress(static_cast<tu_uint32>(std::abs(index)) | 0x80000000); };
    private:
        tu_uint32 u32;
    };

    class ClassAddress {
    public:
        ClassAddress() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit ClassAddress(tu_uint32 u32) : u32(u32) {};
        ClassAddress(const ClassAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const ClassAddress &other) const { return u32 == other.u32; };
        bool operator!=(const ClassAddress &other) const { return u32 != other.u32; };
        static ClassAddress near(int index) { return ClassAddress(static_cast<tu_uint32>(std::abs(index))); };
        static ClassAddress far(int index) { return ClassAddress(static_cast<tu_uint32>(std::abs(index)) | 0x80000000); };
    private:
        tu_uint32 u32;
    };

    class InstanceAddress {
    public:
        InstanceAddress() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit InstanceAddress(tu_uint32 u32) : u32(u32) {};
        InstanceAddress(const InstanceAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const InstanceAddress &other) const { return u32 == other.u32; };
        bool operator!=(const InstanceAddress &other) const { return u32 != other.u32; };
        static InstanceAddress near(int index) { return InstanceAddress(static_cast<tu_uint32>(std::abs(index))); };
        static InstanceAddress far(int index) { return InstanceAddress(static_cast<tu_uint32>(std::abs(index)) | 0x80000000); };
    private:
        tu_uint32 u32;
    };

    class EnumAddress {
    public:
        EnumAddress() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit EnumAddress(tu_uint32 u32) : u32(u32) {};
        EnumAddress(const EnumAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const EnumAddress &other) const { return u32 == other.u32; };
        bool operator!=(const EnumAddress &other) const { return u32 != other.u32; };
        static EnumAddress near(int index) { return EnumAddress(static_cast<tu_uint32>(std::abs(index))); };
        static EnumAddress far(int index) { return EnumAddress(static_cast<tu_uint32>(std::abs(index)) | 0x80000000); };
    private:
        tu_uint32 u32;
    };

    class StructAddress {
    public:
        StructAddress() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit StructAddress(tu_uint32 u32) : u32(u32) {};
        StructAddress(const StructAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const StructAddress &other) const { return u32 == other.u32; };
        bool operator!=(const StructAddress &other) const { return u32 != other.u32; };
        static StructAddress near(int index) { return StructAddress(static_cast<tu_uint32>(std::abs(index))); };
        static StructAddress far(int index) { return StructAddress(static_cast<tu_uint32>(std::abs(index)) | 0x80000000); };
    private:
        tu_uint32 u32;
    };

    class StaticAddress {
    public:
        StaticAddress() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit StaticAddress(tu_uint32 u32) : u32(u32) {};
        StaticAddress(const StaticAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const StaticAddress &other) const { return u32 == other.u32; };
        bool operator!=(const StaticAddress &other) const { return u32 != other.u32; };
        static StaticAddress near(int index) { return StaticAddress(static_cast<tu_uint32>(std::abs(index))); };
        static StaticAddress far(int index) { return StaticAddress(static_cast<tu_uint32>(std::abs(index)) | 0x80000000); };
    private:
        tu_uint32 u32;
    };

    struct TemplateAddress {
    public:
        TemplateAddress() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit TemplateAddress(tu_uint32 u32) : u32(u32) {};
        TemplateAddress(const TemplateAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const TemplateAddress &other) const { return u32 == other.u32; };
        bool operator!=(const TemplateAddress &other) const { return u32 != other.u32; };
        static TemplateAddress near(int index) { return TemplateAddress(static_cast<tu_uint32>(std::abs(index))); };
        static TemplateAddress far(int index) { return TemplateAddress(static_cast<tu_uint32>(std::abs(index)) | 0x80000000); };
    private:
        tu_uint32 u32;
    };

    class TypeAddress {
    public:
        TypeAddress() : u32(lyric_runtime::INVALID_ADDRESS_U32) {};
        explicit TypeAddress(tu_uint32 u32) : u32(u32) {};
        TypeAddress(const TypeAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != lyric_runtime::INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const TypeAddress &other) const { return u32 == other.u32; };
        bool operator!=(const TypeAddress &other) const { return u32 != other.u32; };
        static TypeAddress near(int index) { return TypeAddress(static_cast<tu_uint32>(std::abs(index))); };
        static TypeAddress far(int index) { return TypeAddress(static_cast<tu_uint32>(std::abs(index)) | 0x80000000); };
    private:
        tu_uint32 u32;
    };

    class TypeSignature {

    public:
        TypeSignature();
        explicit TypeSignature(const std::vector<TypeAddress> &signature);
        TypeSignature(const TypeSignature &other);

        bool isValid() const;

        lyric_runtime::TypeComparison compare(const TypeSignature &other) const;
        std::vector<TypeAddress> getSignature() const;
        std::vector<TypeAddress>::const_iterator signatureBegin() const;
        std::vector<TypeAddress>::const_iterator signatureEnd() const;

        bool operator==(const TypeSignature &other) const;
        bool operator!=(const TypeSignature &other) const;

    private:
        std::vector<TypeAddress> m_signature;
    };

    /**
     * A binding of a symbol to a block. A symbol can be bound to multiple blocks, and a symbol can be
     * bound to the same block multiple times as long as each binding has a different name.
     */
    struct SymbolBinding {
        lyric_common::SymbolUrl symbolUrl;
        lyric_common::TypeDef typeDef;
        BindingType bindingType;
        SymbolBinding();
        SymbolBinding(
            const lyric_common::SymbolUrl &symbol,
            const lyric_common::TypeDef &type,
            BindingType bindingType);
    };

    /**
     * A reference to a datum. The datum may exist on the current stack frame (an argument, local, or
     * rest cell), a parent frame (a lexical cell), or in a segment (a static cell).
     */
    struct DataReference {
        lyric_common::SymbolUrl symbolUrl;          /**< The symbol which defines the datum */
        lyric_common::TypeDef typeDef;              /**< The type of the datum */
        ReferenceType referenceType;                /**< the reference type */
        DataReference();
        DataReference(
            const lyric_common::SymbolUrl &symbolUrl,
            const lyric_common::TypeDef &typeDef,
            ReferenceType referenceType);
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

    class CondCasePatch {
    public:
        CondCasePatch();
        CondCasePatch(
            const JumpLabel &predicateLabel,
            const PatchOffset &predicateJump,
            const PatchOffset &consequentJump);
        CondCasePatch(const CondCasePatch &other);

        JumpLabel getPredicateLabel() const;
        PatchOffset getPredicateJump() const;
        PatchOffset getConsequentJump() const;

    private:
        JumpLabel m_predicateLabel;
        PatchOffset m_predicateJump;
        PatchOffset m_consequentJump;
    };

    class MatchCasePatch {
    public:
        MatchCasePatch();
        MatchCasePatch(
            const lyric_common::TypeDef &predicateType,
            const JumpLabel &predicateLabel,
            const PatchOffset &predicateJump,
            const PatchOffset &consequentJump,
            const lyric_common::TypeDef &consequentType);
        MatchCasePatch(const MatchCasePatch &other);

        lyric_common::TypeDef getPredicateType() const;
        JumpLabel getPredicateLabel() const;
        PatchOffset getPredicateJump() const;
        PatchOffset getConsequentJump() const;
        lyric_common::TypeDef getConsequentType() const;

    private:
        lyric_common::TypeDef m_predicateType;
        JumpLabel m_predicateLabel;
        PatchOffset m_predicateJump;
        PatchOffset m_consequentJump;
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
}

#endif // LYRIC_ASSEMBLER_ASSEMBLER_TYPES_H