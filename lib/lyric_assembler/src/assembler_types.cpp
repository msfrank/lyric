
#include <lyric_assembler/assembler_types.h>
#include <tempo_utils/big_endian.h>

std::string
lyric_assembler::fundamentalTypeToString(FundamentalSymbol fundamentalType)
{
    switch (fundamentalType) {
        case FundamentalSymbol::Any:
            return "Any";
        case FundamentalSymbol::Arithmetic:
            return "Arithmetic";
        case FundamentalSymbol::Binding:
            return "Binding";
        case FundamentalSymbol::Bool:
            return "Bool";
        case FundamentalSymbol::Bytes:
            return "Bytes";
        case FundamentalSymbol::Category:
            return "Category";
        case FundamentalSymbol::Call:
            return "Call";
        case FundamentalSymbol::Char:
            return "Char";
        case FundamentalSymbol::Class:
            return "Class";
        case FundamentalSymbol::Comparison:
            return "Comparison";
        case FundamentalSymbol::Concept:
            return "Concept";
        case FundamentalSymbol::Descriptor:
            return "Descriptor";
        case FundamentalSymbol::Enum:
            return "Enum";
        case FundamentalSymbol::Equality:
            return "Equality";
        case FundamentalSymbol::Float:
            return "Float";
        case FundamentalSymbol::Idea:
            return "Idea";
        case FundamentalSymbol::Instance:
            return "Instance";
        case FundamentalSymbol::Int:
            return "Int";
        case FundamentalSymbol::Intrinsic:
            return "Intrinsic";
        case FundamentalSymbol::Iterable:
            return "Iterable";
        case FundamentalSymbol::Iterator:
            return "Iterator";
        case FundamentalSymbol::Map:
            return "Map";
        case FundamentalSymbol::Namespace:
            return "Namespace";
        case FundamentalSymbol::Nil:
            return "Nil";
        case FundamentalSymbol::Object:
            return "Object";
        case FundamentalSymbol::Ordered:
            return "Ordered";
        case FundamentalSymbol::Pair:
            return "Pair";
        case FundamentalSymbol::Proposition:
            return "Proposition";
        case FundamentalSymbol::Record:
            return "Record";
        case FundamentalSymbol::Rest:
            return "Rest";
        case FundamentalSymbol::Seq:
            return "Seq";
        case FundamentalSymbol::Singleton:
            return "Singleton";
        case FundamentalSymbol::Status:
            return "Status";
        case FundamentalSymbol::String:
            return "String";
        case FundamentalSymbol::Struct:
            return "Struct";
        case FundamentalSymbol::Type:
            return "Type";
        case FundamentalSymbol::Undef:
            return "Undef";
        case FundamentalSymbol::Unwrap:
            return "Unwrap";
        case FundamentalSymbol::Url:
            return "Url";

        case FundamentalSymbol::Cancelled:
            return "Cancelled";
        case FundamentalSymbol::InvalidArgument:
            return "InvalidArgument";
        case FundamentalSymbol::DeadlineExceeded:
            return "DeadlineExceeded";
        case FundamentalSymbol::NotFound:
            return "NotFound";
        case FundamentalSymbol::AlreadyExists:
            return "AlreadyExists";
        case FundamentalSymbol::PermissionDenied:
            return "PermissionDenied";
        case FundamentalSymbol::Unauthenticated:
            return "Unauthenticated";
        case FundamentalSymbol::ResourceExhausted:
            return "ResourceExhausted";
        case FundamentalSymbol::FailedPrecondition:
            return "FailedPrecondition";
        case FundamentalSymbol::Aborted:
            return "Aborted";
        case FundamentalSymbol::Unavailable:
            return "Unavailable";
        case FundamentalSymbol::OutOfRange:
            return "OutOfRange";
        case FundamentalSymbol::Unimplemented:
            return "Unimplemented";
        case FundamentalSymbol::Internal:
            return "Internal";
        case FundamentalSymbol::DataLoss:
            return "DataLoss";
        case FundamentalSymbol::Unknown:
            return "Unknown";

        case FundamentalSymbol::Function0:
            return "Function0";
        case FundamentalSymbol::Function1:
            return "Function1";
        case FundamentalSymbol::Function2:
            return "Function2";
        case FundamentalSymbol::Function3:
            return "Function3";
        case FundamentalSymbol::Function4:
            return "Function4";
        case FundamentalSymbol::Function5:
            return "Function5";
        case FundamentalSymbol::Function6:
            return "Function6";
        case FundamentalSymbol::Function7:
            return "Function7";

        case FundamentalSymbol::Tuple1:
            return "Tuple1";
        case FundamentalSymbol::Tuple2:
            return "Tuple2";
        case FundamentalSymbol::Tuple3:
            return "Tuple3";
        case FundamentalSymbol::Tuple4:
            return "Tuple4";
        case FundamentalSymbol::Tuple5:
            return "Tuple5";
        case FundamentalSymbol::Tuple6:
            return "Tuple6";
        case FundamentalSymbol::Tuple7:
            return "Tuple7";

        case FundamentalSymbol::BoolInstance:
            return "BoolInstance";
        case FundamentalSymbol::CharInstance:
            return "CharInstance";
        case FundamentalSymbol::FloatInstance:
            return "FloatInstance";
        case FundamentalSymbol::IntInstance:
            return "IntInstance";
        case FundamentalSymbol::BytesInstance:
            return "BytesInstance";
        case FundamentalSymbol::StringInstance:
            return "StringInstance";
        case FundamentalSymbol::UrlInstance:
            return "UrlInstance";

        case FundamentalSymbol::Tuple1Instance:
            return "Tuple1Instance";
        case FundamentalSymbol::Tuple2Instance:
            return "Tuple2Instance";
        case FundamentalSymbol::Tuple3Instance:
            return "Tuple3Instance";
        case FundamentalSymbol::Tuple4Instance:
            return "Tuple4Instance";
        case FundamentalSymbol::Tuple5Instance:
            return "Tuple5Instance";
        case FundamentalSymbol::Tuple6Instance:
            return "Tuple6Instance";
        case FundamentalSymbol::Tuple7Instance:
            return "Tuple7Instance";

        default:
            TU_UNREACHABLE();
    }
}

lyric_common::SymbolPath
lyric_assembler::fundamentalTypeToSymbolPath(FundamentalSymbol fundamentalType)
{
    auto name = fundamentalTypeToString(fundamentalType);
    if (name.empty())
        return {};
    return lyric_common::SymbolPath({name});
}

lyric_assembler::TypeSignature::TypeSignature()
    : m_signature()
{
}

lyric_assembler::TypeSignature::TypeSignature(const std::vector<const TypeHandle *> &signature)
    : m_signature(signature)
{
}

lyric_assembler::TypeSignature::TypeSignature(const TypeSignature &other)
    : m_signature(other.m_signature)
{
}

bool
lyric_assembler::TypeSignature::isValid() const
{
    return !m_signature.empty();
}

std::vector<const lyric_assembler::TypeHandle *>
lyric_assembler::TypeSignature::getSignature() const
{
    return m_signature;
}

std::vector<const lyric_assembler::TypeHandle *>::const_iterator
lyric_assembler::TypeSignature::signatureBegin() const
{
    return m_signature.cbegin();
}

std::vector<const lyric_assembler::TypeHandle *>::const_iterator
lyric_assembler::TypeSignature::signatureEnd() const
{
    return m_signature.cend();
}

lyric_runtime::TypeComparison
lyric_assembler::TypeSignature::compare(const TypeSignature &other) const
{
    if (m_signature.empty() || other.m_signature.empty())
        return lyric_runtime::TypeComparison::DISJOINT;

    auto othersize = other.m_signature.size();
    for (tu_uint32 i = 0; i < m_signature.size(); i++) {
        if (i == othersize)
            return lyric_runtime::TypeComparison::EXTENDS;     // other is subtype
        if (m_signature[i] != other.m_signature[i])
            return lyric_runtime::TypeComparison::DISJOINT;    // no direct type relationship
    }
    // if signature sizes are equal, then types are equal, otherwise other is supertype
    return m_signature.size() == othersize? lyric_runtime::TypeComparison::EQUAL : lyric_runtime::TypeComparison::SUPER;
}

bool
lyric_assembler::TypeSignature::operator==(const TypeSignature &other) const
{
    return m_signature == other.m_signature;
}

bool
lyric_assembler::TypeSignature::operator!=(const TypeSignature &other) const
{
    return !(this->operator==(other));
}

lyric_assembler::SymbolBinding::SymbolBinding()
    : bindingType(BindingType::Invalid),
      symbolUrl(),
      typeDef()
{
}

lyric_assembler::SymbolBinding::SymbolBinding(
    BindingType bindingType,
    const lyric_common::SymbolUrl &symbolUrl,
    const lyric_common::TypeDef &typeDef)
    : bindingType(bindingType),
      symbolUrl(symbolUrl),
      typeDef(typeDef)
{
    TU_ASSERT (bindingType != BindingType::Invalid);
    TU_ASSERT (symbolUrl.isValid());
    TU_ASSERT (typeDef.isValid());
}

lyric_assembler::DataReference::DataReference()
    : referenceType(ReferenceType::Invalid),
      symbolUrl(),
      typeDef()
{
}

lyric_assembler::DataReference::DataReference(
    ReferenceType referenceType,
    const lyric_common::SymbolUrl &symbolUrl,
    const lyric_common::TypeDef &typeDef)
    : referenceType(referenceType),
      symbolUrl(symbolUrl),
      typeDef(typeDef)
{
    TU_ASSERT (referenceType != ReferenceType::Invalid);
    TU_ASSERT (symbolUrl.isValid());
    TU_ASSERT (typeDef.isValid());
}

lyric_assembler::ActionMethod::ActionMethod()
    : methodAction()
{
}

lyric_assembler::ActionMethod::ActionMethod(const lyric_common::SymbolUrl &methodAction)
    : methodAction(methodAction)
{
    TU_ASSERT (methodAction.isValid());
}

lyric_assembler::BoundMethod::BoundMethod()
    : methodCall(),
      hidden(false),
      final(false)
{
}

lyric_assembler::BoundMethod::BoundMethod(
    const lyric_common::SymbolUrl &methodCall,
    bool hidden,
    bool final)
    : methodCall(methodCall),
      hidden(hidden),
      final(final)
{
    TU_ASSERT (methodCall.isValid());
}

lyric_assembler::ExtensionMethod::ExtensionMethod()
{
}

lyric_assembler::ExtensionMethod::ExtensionMethod(
    const lyric_common::SymbolUrl &methodCall,
    const lyric_common::SymbolUrl &methodAction)
    : methodCall(methodCall),
      methodAction(methodAction)
{
    TU_ASSERT (methodCall.isValid());
    TU_ASSERT (methodAction.isValid());
}

lyric_assembler::CondWhenPatch::CondWhenPatch()
    : m_predicateLabel(),
      m_predicateJump(),
      m_consequentJump()
{
}

lyric_assembler::CondWhenPatch::CondWhenPatch(
    const JumpLabel &predicateLabel,
    const JumpTarget &predicateJump,
    const JumpTarget &consequentJump)
    : m_predicateLabel(predicateLabel),
      m_predicateJump(predicateJump),
      m_consequentJump(consequentJump)
{
}

lyric_assembler::CondWhenPatch::CondWhenPatch(const CondWhenPatch &other)
    : m_predicateLabel(other.m_predicateLabel),
      m_predicateJump(other.m_predicateJump),
      m_consequentJump(other.m_consequentJump)
{
}

lyric_assembler::JumpLabel
lyric_assembler::CondWhenPatch::getPredicateLabel() const
{
    return m_predicateLabel;
}

lyric_assembler::JumpTarget
lyric_assembler::CondWhenPatch::getPredicateJump() const
{
    return m_predicateJump;
}

lyric_assembler::JumpTarget
lyric_assembler::CondWhenPatch::getConsequentJump() const
{
    return m_consequentJump;
}

lyric_assembler::MatchWhenPatch::MatchWhenPatch()
    : m_predicateType(),
      m_predicateLabel(),
      m_predicateJump(),
      m_consequentJump(),
      m_consequentType()
{
}

lyric_assembler::MatchWhenPatch::MatchWhenPatch(
    const lyric_common::TypeDef &predicateType,
    const JumpLabel &predicateLabel,
    const JumpTarget &predicateJump,
    const JumpTarget &consequentJump,
    const lyric_common::TypeDef &consequentType)
    : m_predicateType(predicateType),
      m_predicateLabel(predicateLabel),
      m_predicateJump(predicateJump),
      m_consequentJump(consequentJump),
      m_consequentType(consequentType)
{
}

lyric_assembler::MatchWhenPatch::MatchWhenPatch(const MatchWhenPatch &other)
    : m_predicateType(other.m_predicateType),
      m_predicateLabel(other.m_predicateLabel),
      m_predicateJump(other.m_predicateJump),
      m_consequentJump(other.m_consequentJump),
      m_consequentType(other.m_consequentType)
{
}

lyric_common::TypeDef
lyric_assembler::MatchWhenPatch::MatchWhenPatch::getPredicateType() const
{
    return m_predicateType;
}

lyric_assembler::JumpLabel
lyric_assembler::MatchWhenPatch::MatchWhenPatch::getPredicateLabel() const
{
    return m_predicateLabel;
}

lyric_assembler::JumpTarget
lyric_assembler::MatchWhenPatch::MatchWhenPatch::getPredicateJump() const
{
    return m_predicateJump;
}

lyric_assembler::JumpTarget
lyric_assembler::MatchWhenPatch::MatchWhenPatch::getConsequentJump() const
{
    return m_consequentJump;
}

lyric_common::TypeDef
lyric_assembler::MatchWhenPatch::MatchWhenPatch::getConsequentType() const
{
    return m_consequentType;
}

lyric_assembler::ImportRef::ImportRef()
{
}

lyric_assembler::ImportRef::ImportRef(const lyric_common::SymbolPath &path)
    : m_path(path),
      m_name(path.getName())
{
    TU_ASSERT (m_path.isValid());
}

lyric_assembler::ImportRef::ImportRef(const lyric_common::SymbolPath &path, std::string_view alias)
    : m_path(path),
      m_name(alias)
{
    TU_ASSERT (m_path.isValid());
    TU_ASSERT (!m_name.empty());
}

lyric_assembler::ImportRef::ImportRef(const ImportRef &other)
    : m_path(other.m_path),
      m_name(other.m_name)
{
}

bool
lyric_assembler::ImportRef::isValid() const
{
    return m_path.isValid();
}

lyric_common::SymbolPath
lyric_assembler::ImportRef::getPath() const
{
    return m_path;
}

std::string
lyric_assembler::ImportRef::getName() const
{
    return m_name;
}

bool lyric_assembler::ImportRef::operator==(const ImportRef &other) const
{
    return m_path == other.m_path;
}

bool
lyric_assembler::ImportRef::operator!=(const ImportRef &other) const
{
    return m_path != other.m_path;
}
