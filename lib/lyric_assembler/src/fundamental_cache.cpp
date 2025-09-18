
#include <lyric_assembler/fundamental_cache.h>

lyric_assembler::FundamentalCache::FundamentalCache(const lyric_common::ModuleLocation &preludeLocation)
    : m_preludeLocation(preludeLocation)
{
    TU_ASSERT (m_preludeLocation.isValid());

    // cache symbol urls for fundamentals
    m_fundamentalAny = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Any));
    m_fundamentalArithmetic = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Arithmetic));
    m_fundamentalBinding = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Binding));
    m_fundamentalBool = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Bool));
    m_fundamentalBytes = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Bytes));
    m_fundamentalCall = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Call));
    m_fundamentalCategory = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Category));
    m_fundamentalChar = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Char));
    m_fundamentalClass = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Class));
    m_fundamentalComparison = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Comparison));
    m_fundamentalConcept = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Concept));
    m_fundamentalDescriptor = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Descriptor));
    m_fundamentalEnum = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Enum));
    m_fundamentalEquality = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Equality));
    m_fundamentalFloat = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Float));
    m_fundamentalIdea = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Idea));
    m_fundamentalInstance = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Instance));
    m_fundamentalInt = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Int));
    m_fundamentalIntrinsic = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Intrinsic));
    m_fundamentalIterable = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Iterable));
    m_fundamentalIterator = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Iterator));
    m_fundamentalMap = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Map));
    m_fundamentalNamespace = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Namespace));
    m_fundamentalNil = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Nil));
    m_fundamentalObject = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Object));
    m_fundamentalOrdered = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Ordered));
    m_fundamentalPair = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Pair));
    m_fundamentalProposition = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Proposition));
    m_fundamentalRecord = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Record));
    m_fundamentalRest = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Rest));
    m_fundamentalSeq = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Seq));
    m_fundamentalSingleton = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Singleton));
    m_fundamentalStatus = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Status));
    m_fundamentalString = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::String));
    m_fundamentalStruct = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Struct));
    m_fundamentalType = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Type));
    m_fundamentalUndef = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Undef));
    m_fundamentalUnwrap = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Unwrap));
    m_fundamentalUrl = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Url));

    // cache symbol urls for statuses
    m_fundamentalCancelled = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Cancelled));
    m_fundamentalInvalidArgument = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::InvalidArgument));
    m_fundamentalDeadlineExceeded = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::DeadlineExceeded));
    m_fundamentalNotFound = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::NotFound));
    m_fundamentalAlreadyExists = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::AlreadyExists));
    m_fundamentalPermissionDenied = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::PermissionDenied));
    m_fundamentalUnauthenticated = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Unauthenticated));
    m_fundamentalResourceExhausted = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::ResourceExhausted));
    m_fundamentalFailedPrecondition = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::FailedPrecondition));
    m_fundamentalAborted = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Aborted));
    m_fundamentalUnavailable = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Unavailable));
    m_fundamentalOutOfRange = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::OutOfRange));
    m_fundamentalUnimplemented = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Unimplemented));
    m_fundamentalInternal = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Internal));
    m_fundamentalDataLoss = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::DataLoss));
    m_fundamentalUnknown = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::Unknown));

    // cache symbol urls for companion instances
    m_fundamentalBoolInstance = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::BoolInstance));
    m_fundamentalCharInstance = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::CharInstance));
    m_fundamentalFloatInstance = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::FloatInstance));
    m_fundamentalIntInstance = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::IntInstance));
    m_fundamentalBytesInstance = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::BytesInstance));
    m_fundamentalStringInstance = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::StringInstance));
    m_fundamentalUrlInstance = lyric_common::SymbolUrl(preludeLocation,
        fundamentalTypeToSymbolPath(FundamentalSymbol::UrlInstance));

    // cache symbol urls for Function0 through Function7
    m_fundamentalFunction.resize(kNumFunctionClasses);
    m_fundamentalFunction[0] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Function0));
    m_fundamentalFunction[1] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Function1));
    m_fundamentalFunction[2] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Function2));
    m_fundamentalFunction[3] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Function3));
    m_fundamentalFunction[4] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Function4));
    m_fundamentalFunction[5] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Function5));
    m_fundamentalFunction[6] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Function6));
    m_fundamentalFunction[7] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Function7));

    // cache symbol urls for Tuple1 through TupleN
    m_fundamentalTuple.resize(kNumTupleClasses);
    m_fundamentalTuple[0] = {};     // no such thing as Tuple0 but insert into the array
    m_fundamentalTuple[1] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Tuple1));
    m_fundamentalTuple[2] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Tuple2));
    m_fundamentalTuple[3] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Tuple3));
    m_fundamentalTuple[4] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Tuple4));
    m_fundamentalTuple[5] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Tuple5));
    m_fundamentalTuple[6] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Tuple6));
    m_fundamentalTuple[7] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Tuple7));

    // cache symbol urls for Tuple1Instance through TupleNInstance
    m_fundamentalTupleInstance.resize(kNumTupleClasses);
    m_fundamentalTupleInstance[0] = {};     // no such thing as Tuple0 but insert into the array
    m_fundamentalTupleInstance[1] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Tuple1Instance));
    m_fundamentalTupleInstance[2] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Tuple2Instance));
    m_fundamentalTupleInstance[3] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Tuple3Instance));
    m_fundamentalTupleInstance[4] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Tuple4Instance));
    m_fundamentalTupleInstance[5] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Tuple5Instance));
    m_fundamentalTupleInstance[6] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Tuple6Instance));
    m_fundamentalTupleInstance[7] = lyric_common::SymbolUrl(
        preludeLocation, fundamentalTypeToSymbolPath(FundamentalSymbol::Tuple7Instance));
}

lyric_common::ModuleLocation
lyric_assembler::FundamentalCache::getPreludeLocation() const
{
    return m_preludeLocation;
}

lyric_common::TypeDef
lyric_assembler::FundamentalCache::getFundamentalType(FundamentalSymbol fundamental) const
{
    auto fundamentalUrl = getFundamentalUrl(fundamental);
    if (!fundamentalUrl.isValid())
        return {};
    return lyric_common::TypeDef::forConcrete(fundamentalUrl).orElseThrow();
}

lyric_common::SymbolUrl
lyric_assembler::FundamentalCache::getFundamentalUrl(FundamentalSymbol fundamental) const
{
    switch (fundamental) {
        case FundamentalSymbol::Any:
            return m_fundamentalAny;
        case FundamentalSymbol::Arithmetic:
            return m_fundamentalArithmetic;
        case FundamentalSymbol::Binding:
            return m_fundamentalBinding;
        case FundamentalSymbol::Bool:
            return m_fundamentalBool;
        case FundamentalSymbol::Bytes:
            return m_fundamentalBytes;
        case FundamentalSymbol::Call:
            return m_fundamentalCall;
        case FundamentalSymbol::Category:
            return m_fundamentalCategory;
        case FundamentalSymbol::Char:
            return m_fundamentalChar;
        case FundamentalSymbol::Class:
            return m_fundamentalClass;
        case FundamentalSymbol::Comparison:
            return m_fundamentalComparison;
        case FundamentalSymbol::Concept:
            return m_fundamentalConcept;
        case FundamentalSymbol::Descriptor:
            return m_fundamentalDescriptor;
        case FundamentalSymbol::Enum:
            return m_fundamentalEnum;
        case FundamentalSymbol::Equality:
            return m_fundamentalEquality;
        case FundamentalSymbol::Float:
            return m_fundamentalFloat;
        case FundamentalSymbol::Idea:
            return m_fundamentalIdea;
        case FundamentalSymbol::Instance:
            return m_fundamentalInstance;
        case FundamentalSymbol::Int:
            return m_fundamentalInt;
        case FundamentalSymbol::Intrinsic:
            return m_fundamentalIntrinsic;
        case FundamentalSymbol::Iterable:
            return m_fundamentalIterable;
        case FundamentalSymbol::Iterator:
            return m_fundamentalIterator;
        case FundamentalSymbol::Map:
            return m_fundamentalMap;
        case FundamentalSymbol::Namespace:
            return m_fundamentalNamespace;
        case FundamentalSymbol::Nil:
            return m_fundamentalNil;
        case FundamentalSymbol::Object:
            return m_fundamentalObject;
        case FundamentalSymbol::Ordered:
            return m_fundamentalOrdered;
        case FundamentalSymbol::Pair:
            return m_fundamentalPair;
        case FundamentalSymbol::Proposition:
            return m_fundamentalProposition;
        case FundamentalSymbol::Record:
            return m_fundamentalRecord;
        case FundamentalSymbol::Rest:
            return m_fundamentalRest;
        case FundamentalSymbol::Seq:
            return m_fundamentalSeq;
        case FundamentalSymbol::Singleton:
            return m_fundamentalSingleton;
        case FundamentalSymbol::Status:
            return m_fundamentalStatus;
        case FundamentalSymbol::String:
            return m_fundamentalString;
        case FundamentalSymbol::Struct:
            return m_fundamentalStruct;
        case FundamentalSymbol::Type:
            return m_fundamentalType;
        case FundamentalSymbol::Undef:
            return m_fundamentalUndef;
        case FundamentalSymbol::Unwrap:
            return m_fundamentalUnwrap;
        case FundamentalSymbol::Url:
            return m_fundamentalUrl;

        case FundamentalSymbol::Cancelled:
            return m_fundamentalCancelled;
        case FundamentalSymbol::InvalidArgument:
            return m_fundamentalInvalidArgument;
        case FundamentalSymbol::DeadlineExceeded:
            return m_fundamentalDeadlineExceeded;
        case FundamentalSymbol::NotFound:
            return m_fundamentalNotFound;
        case FundamentalSymbol::AlreadyExists:
            return m_fundamentalAlreadyExists;
        case FundamentalSymbol::PermissionDenied:
            return m_fundamentalPermissionDenied;
        case FundamentalSymbol::Unauthenticated:
            return m_fundamentalUnauthenticated;
        case FundamentalSymbol::ResourceExhausted:
            return m_fundamentalResourceExhausted;
        case FundamentalSymbol::FailedPrecondition:
            return m_fundamentalFailedPrecondition;
        case FundamentalSymbol::Aborted:
            return m_fundamentalAborted;
        case FundamentalSymbol::Unavailable:
            return m_fundamentalUnavailable;
        case FundamentalSymbol::OutOfRange:
            return m_fundamentalOutOfRange;
        case FundamentalSymbol::Unimplemented:
            return m_fundamentalUnimplemented;
        case FundamentalSymbol::Internal:
            return m_fundamentalInternal;
        case FundamentalSymbol::DataLoss:
            return m_fundamentalDataLoss;
        case FundamentalSymbol::Unknown:
            return m_fundamentalUnknown;

        case FundamentalSymbol::Function0:
            return m_fundamentalFunction.at(0);
        case FundamentalSymbol::Function1:
            return m_fundamentalFunction.at(1);
        case FundamentalSymbol::Function2:
            return m_fundamentalFunction.at(2);
        case FundamentalSymbol::Function3:
            return m_fundamentalFunction.at(3);
        case FundamentalSymbol::Function4:
            return m_fundamentalFunction.at(4);
        case FundamentalSymbol::Function5:
            return m_fundamentalFunction.at(5);
        case FundamentalSymbol::Function6:
            return m_fundamentalFunction.at(6);
        case FundamentalSymbol::Function7:
            return m_fundamentalFunction.at(7);

        case FundamentalSymbol::Tuple1:
            return m_fundamentalTuple.at(1);
        case FundamentalSymbol::Tuple2:
            return m_fundamentalTuple.at(2);
        case FundamentalSymbol::Tuple3:
            return m_fundamentalTuple.at(3);
        case FundamentalSymbol::Tuple4:
            return m_fundamentalTuple.at(4);
        case FundamentalSymbol::Tuple5:
            return m_fundamentalTuple.at(5);
        case FundamentalSymbol::Tuple6:
            return m_fundamentalTuple.at(6);
        case FundamentalSymbol::Tuple7:
            return m_fundamentalTuple.at(7);


        case FundamentalSymbol::BoolInstance:
            return m_fundamentalBoolInstance;
        case FundamentalSymbol::CharInstance:
            return m_fundamentalCharInstance;
        case FundamentalSymbol::FloatInstance:
            return m_fundamentalFloatInstance;
        case FundamentalSymbol::IntInstance:
            return m_fundamentalIntInstance;
        case FundamentalSymbol::BytesInstance:
            return m_fundamentalBytesInstance;
        case FundamentalSymbol::StringInstance:
            return m_fundamentalStringInstance;
        case FundamentalSymbol::UrlInstance:
            return m_fundamentalUrlInstance;

        case FundamentalSymbol::Tuple1Instance:
            return m_fundamentalTupleInstance.at(1);
        case FundamentalSymbol::Tuple2Instance:
            return m_fundamentalTupleInstance.at(2);
        case FundamentalSymbol::Tuple3Instance:
            return m_fundamentalTupleInstance.at(3);
        case FundamentalSymbol::Tuple4Instance:
            return m_fundamentalTupleInstance.at(4);
        case FundamentalSymbol::Tuple5Instance:
            return m_fundamentalTupleInstance.at(5);
        case FundamentalSymbol::Tuple6Instance:
            return m_fundamentalTupleInstance.at(6);
        case FundamentalSymbol::Tuple7Instance:
            return m_fundamentalTupleInstance.at(7);

        default:
            return {};
    }
}

lyric_common::SymbolUrl
lyric_assembler::FundamentalCache::getFundamentalUrl(const lyric_runtime::LiteralCell &literalCell) const
{
    switch (literalCell.type) {
        case lyric_runtime::LiteralCellType::NIL:
            return getFundamentalUrl(FundamentalSymbol::Nil);
        case lyric_runtime::LiteralCellType::UNDEF:
            return getFundamentalUrl(FundamentalSymbol::Undef);
        case lyric_runtime::LiteralCellType::BOOL:
            return getFundamentalUrl(FundamentalSymbol::Bool);
        case lyric_runtime::LiteralCellType::I64:
            return getFundamentalUrl(FundamentalSymbol::Int);
        case lyric_runtime::LiteralCellType::DBL:
            return getFundamentalUrl(FundamentalSymbol::Float);
        case lyric_runtime::LiteralCellType::CHAR32:
            return getFundamentalUrl(FundamentalSymbol::Char);
        case lyric_runtime::LiteralCellType::INVALID:
        default:
            return {};
    }
}

lyric_common::SymbolUrl
lyric_assembler::FundamentalCache::getFunctionUrl(int arity) const
{
    if (0 <= arity && std::cmp_less(arity, m_fundamentalFunction.size()))
        return m_fundamentalFunction.at(arity);
    return {};
}

lyric_common::SymbolUrl
lyric_assembler::FundamentalCache::getTupleUrl(int arity) const
{
    if (0 <= arity && std::cmp_less(arity, m_fundamentalTuple.size()))
        return m_fundamentalTuple.at(arity);
    return {};
}
