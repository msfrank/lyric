#ifndef LYRIC_ASSEMBLER_FUNDAMENTAL_CACHE_H
#define LYRIC_ASSEMBLER_FUNDAMENTAL_CACHE_H

#include <lyric_common/assembly_location.h>
#include <lyric_common/symbol_url.h>

#include "assembler_tracer.h"
#include "assembler_types.h"

namespace lyric_assembler {

    class FundamentalCache {
    public:
        FundamentalCache(const lyric_common::AssemblyLocation &preludeLocation, AssemblerTracer *tracer);

        lyric_common::AssemblyLocation getPreludeLocation() const;

        lyric_common::TypeDef getFundamentalType(FundamentalSymbol fundamental) const;
        lyric_common::SymbolUrl getFundamentalUrl(FundamentalSymbol fundamental) const;
        lyric_common::SymbolUrl getFundamentalUrl(const lyric_runtime::LiteralCell &literalCell) const;
        lyric_common::SymbolUrl getFunctionUrl(int arity) const;
        lyric_common::SymbolUrl getTupleUrl(int arity) const;

    private:
        lyric_common::AssemblyLocation m_preludeLocation;
        AssemblerTracer *m_tracer;

        lyric_common::SymbolUrl m_fundamentalAny;
        lyric_common::SymbolUrl m_fundamentalArithmetic;
        lyric_common::SymbolUrl m_fundamentalBool;
        lyric_common::SymbolUrl m_fundamentalCall;
        lyric_common::SymbolUrl m_fundamentalCategory;
        lyric_common::SymbolUrl m_fundamentalChar;
        lyric_common::SymbolUrl m_fundamentalClass;
        lyric_common::SymbolUrl m_fundamentalComparison;
        lyric_common::SymbolUrl m_fundamentalConcept;
        lyric_common::SymbolUrl m_fundamentalEnum;
        lyric_common::SymbolUrl m_fundamentalEquality;
        lyric_common::SymbolUrl m_fundamentalFloat;
        lyric_common::SymbolUrl m_fundamentalIdea;
        lyric_common::SymbolUrl m_fundamentalInt;
        lyric_common::SymbolUrl m_fundamentalInstance;
        lyric_common::SymbolUrl m_fundamentalIntrinsic;
        lyric_common::SymbolUrl m_fundamentalIterator;
        lyric_common::SymbolUrl m_fundamentalMap;
        lyric_common::SymbolUrl m_fundamentalNamespace;
        lyric_common::SymbolUrl m_fundamentalNil;
        lyric_common::SymbolUrl m_fundamentalObject;
        lyric_common::SymbolUrl m_fundamentalOrdered;
        lyric_common::SymbolUrl m_fundamentalPair;
        lyric_common::SymbolUrl m_fundamentalPresent;
        lyric_common::SymbolUrl m_fundamentalProposition;
        lyric_common::SymbolUrl m_fundamentalRecord;
        lyric_common::SymbolUrl m_fundamentalSeq;
        lyric_common::SymbolUrl m_fundamentalSingleton;
        lyric_common::SymbolUrl m_fundamentalStatus;
        lyric_common::SymbolUrl m_fundamentalString;
        lyric_common::SymbolUrl m_fundamentalStruct;
        lyric_common::SymbolUrl m_fundamentalUnwrap;
        lyric_common::SymbolUrl m_fundamentalUrl;
        lyric_common::SymbolUrl m_fundamentalUtf8;

        lyric_common::SymbolUrl m_fundamentalOk;
        lyric_common::SymbolUrl m_fundamentalCancelled;
        lyric_common::SymbolUrl m_fundamentalInvalidArgument;
        lyric_common::SymbolUrl m_fundamentalDeadlineExceeded;
        lyric_common::SymbolUrl m_fundamentalNotFound;
        lyric_common::SymbolUrl m_fundamentalAlreadyExists;
        lyric_common::SymbolUrl m_fundamentalPermissionDenied;
        lyric_common::SymbolUrl m_fundamentalUnauthenticated;
        lyric_common::SymbolUrl m_fundamentalResourceExhausted;
        lyric_common::SymbolUrl m_fundamentalFailedPrecondition;
        lyric_common::SymbolUrl m_fundamentalAborted;
        lyric_common::SymbolUrl m_fundamentalUnavailable;
        lyric_common::SymbolUrl m_fundamentalOutOfRange;
        lyric_common::SymbolUrl m_fundamentalUnimplemented;
        lyric_common::SymbolUrl m_fundamentalInternal;
        lyric_common::SymbolUrl m_fundamentalDataLoss;
        lyric_common::SymbolUrl m_fundamentalUnknown;

        std::vector<lyric_common::SymbolUrl> m_fundamentalFunction;
        std::vector<lyric_common::SymbolUrl> m_fundamentalTuple;

        lyric_common::SymbolUrl m_fundamentalBoolInstance;
        lyric_common::SymbolUrl m_fundamentalCharInstance;
        lyric_common::SymbolUrl m_fundamentalFloatInstance;
        lyric_common::SymbolUrl m_fundamentalIntInstance;
        lyric_common::SymbolUrl m_fundamentalStringInstance;
        lyric_common::SymbolUrl m_fundamentalUrlInstance;
        std::vector<lyric_common::SymbolUrl> m_fundamentalTupleInstance;
    };
}

#endif // LYRIC_ASSEMBLER_FUNDAMENTAL_CACHE_H
