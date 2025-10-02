
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/check_handle.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <tempo_utils/log_stream.h>

/**
 * Allocate a new empty ProcHandle.  This is used during compilation to create
 * a placeholder proc when importing a call from another assembly.
 */
lyric_assembler::ProcHandle::ProcHandle(
    const lyric_common::SymbolUrl &activation,
    ObjectState *state)
    : m_activation(activation),
      m_numListParameters(0),
      m_numNamedParameters(0),
      m_hasRestParameter(false),
      m_numLocals(0),
      m_state(state)
{
    TU_ASSERT (state != nullptr);
    m_code = std::make_unique<ProcBuilder>(this, state);
}

/**
 * Allocate a new ProcHandle for the entry call.
 *
 * @param activation
 * @param parent
 * @param state
 */
lyric_assembler::ProcHandle::ProcHandle(
    const lyric_common::SymbolUrl &activation,
    BlockHandle *parent,
    ObjectState *state)
    : m_activation(activation),
      m_numListParameters(0),
      m_numNamedParameters(0),
      m_hasRestParameter(false),
      m_numLocals(0),
      m_state(state)
{
    TU_ASSERT (state != nullptr);
    TU_ASSERT (parent != nullptr);
    m_code = std::make_unique<ProcBuilder>(this, state);
    absl::flat_hash_map<std::string,SymbolBinding> initialBindings;
    m_block = std::make_unique<BlockHandle>(initialBindings, this, parent, state);
}

/**
 *
 * @param parameters
 * @param state
 * @param parent
 */
lyric_assembler::ProcHandle::ProcHandle(
    const lyric_common::SymbolUrl &activation,
    const absl::flat_hash_map<std::string,SymbolBinding> &initialBindings,
    tu_uint8 numListParameters,
    tu_uint8 numNamedParameters,
    bool hasRestParameter,
    ObjectState *state,
    BlockHandle *parent)
    : m_activation(activation),
      m_numListParameters(numListParameters),
      m_numNamedParameters(numNamedParameters),
      m_hasRestParameter(hasRestParameter),
      m_numLocals(0),
      m_state(state)
{
    TU_ASSERT (state != nullptr);
    TU_ASSERT (parent != nullptr);
    m_code = std::make_unique<ProcBuilder>(this, state);
    m_block = std::make_unique<BlockHandle>(initialBindings, this, parent, state);
}

lyric_assembler::BlockHandle *
lyric_assembler::ProcHandle::procBlock()
{
    return m_block.get();
}

const lyric_assembler::BlockHandle *
lyric_assembler::ProcHandle::procBlock() const
{
    return m_block.get();
}

lyric_assembler::ProcBuilder *
lyric_assembler::ProcHandle::procCode()
{
    return m_code.get();
}

const lyric_assembler::ProcBuilder *
lyric_assembler::ProcHandle::procCode() const
{
    return m_code.get();
}

lyric_common::SymbolUrl
lyric_assembler::ProcHandle::getActivationUrl() const
{
    return m_activation;
}

const lyric_assembler::CallSymbol *
lyric_assembler::ProcHandle::getActivationCall() const
{
    auto *symbolCache = m_state->symbolCache();
    auto sym = symbolCache->getSymbolOrNull(m_activation);
    if (sym == nullptr || sym->getSymbolType() != SymbolType::CALL)
        return nullptr;
    return cast_symbol_to_call(sym);
}

int
lyric_assembler::ProcHandle::getArity() const
{
    return m_numListParameters + m_numNamedParameters;
}

lyric_assembler::LocalOffset
lyric_assembler::ProcHandle::allocateLocal()
{
    auto index = m_numLocals++;
    return LocalOffset(index);
}

int
lyric_assembler::ProcHandle::numLocals() const
{
    return m_numLocals;
}

int
lyric_assembler::ProcHandle::numListParameters() const
{
    return m_numListParameters;
}

int
lyric_assembler::ProcHandle::numNamedParameters() const
{
    return m_numNamedParameters;
}

bool
lyric_assembler::ProcHandle::hasRestParameter() const
{
    return m_hasRestParameter;
}

lyric_assembler::LexicalOffset
lyric_assembler::ProcHandle::allocateLexical(
    LexicalTarget lexicalTarget,
    uint32_t targetOffset,
    CallSymbol *activationCall)
{
    auto index = m_lexicals.size();
    ProcLexical lexical;
    lexical.lexicalTarget = lexicalTarget;
    lexical.targetOffset = targetOffset;
    lexical.activationCall = activationCall;
    m_lexicals.push_back(lexical);
    return LexicalOffset(index);
}

std::vector<lyric_assembler::ProcLexical>::const_iterator
lyric_assembler::ProcHandle::lexicalsBegin() const
{
    return m_lexicals.cbegin();
}

std::vector<lyric_assembler::ProcLexical>::const_iterator
lyric_assembler::ProcHandle::lexicalsEnd() const
{
    return m_lexicals.cend();
}

int
lyric_assembler::ProcHandle::numLexicals() const
{
    return m_lexicals.size();
}

tempo_utils::Result<lyric_assembler::CheckHandle *>
lyric_assembler::ProcHandle::declareCheck(const JumpLabel &checkStart)
{
    auto check = std::make_unique<CheckHandle>(checkStart, this, m_state);
    auto *checkPtr = check.get();
    m_checks.push_back(std::move(check));
    return checkPtr;
}

int
lyric_assembler::ProcHandle::numChecks() const
{
    return m_checks.size();
}

tempo_utils::Result<lyric_assembler::CleanupHandle *>
lyric_assembler::ProcHandle::declareCleanup(const JumpLabel &cleanupStart)
{
    auto cleanup = std::make_unique<CleanupHandle>(cleanupStart, m_state);
    auto *cleanupPtr = cleanup.get();
    m_cleanups.push_back(std::move(cleanup));
    return cleanupPtr;
}

int
lyric_assembler::ProcHandle::numCleanups() const
{
    return m_cleanups.size();
}

void
lyric_assembler::ProcHandle::putExitType(const lyric_common::TypeDef &exitType)
{
    TU_ASSERT (exitType.isValid());
    m_exitTypes.insert(exitType);
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::ProcHandle::exitTypesBegin() const
{
    return m_exitTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::ProcHandle::exitTypesEnd() const
{
    return m_exitTypes.cend();
}
