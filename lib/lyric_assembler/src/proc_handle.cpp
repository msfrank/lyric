
#include <lyric_assembler/proc_handle.h>
#include <tempo_utils/log_stream.h>

/**
 * Allocate a new empty ProcHandle.  this is used during compilation to create
 * a placeholder proc when importing a call from another assembly.
 */
lyric_assembler::ProcHandle::ProcHandle(const lyric_common::SymbolUrl &activation)
    : m_activation(activation),
      m_numListParameters(0),
      m_numNamedParameters(0),
      m_hasRestParameter(false),
      m_block(nullptr),
      m_numLocals(0)
{
}

/**
 * Allocate a ProcHandle with the specified bytecode.  this is used during compilation
 * to create a proc with inline code when importing a call from another assembly.
 *
 * @param bytecode
 */
lyric_assembler::ProcHandle::ProcHandle(
    const lyric_common::SymbolUrl &activation,
    const std::vector<tu_uint8> &bytecode)
    : m_activation(activation),
      m_numListParameters(0),
      m_numNamedParameters(0),
      m_hasRestParameter(false),
      m_code(bytecode),
      m_block(nullptr),
      m_numLocals(0)
{
    TU_ASSERT (m_code.bytecodeSize() > 0);
}

/**
 * Allocate a ProcHandle with the specified bytecode and locals arity. this is used during
 * compilation to create a proc for the $entry symbol.
 */
lyric_assembler::ProcHandle::ProcHandle(
    const lyric_common::SymbolUrl &activation,
    const std::vector<tu_uint8> &bytecode,
    int numLocals)
    : m_activation(activation),
      m_numListParameters(0),
      m_numNamedParameters(0),
      m_hasRestParameter(false),
      m_code(bytecode),
      m_block(nullptr),
      m_numLocals(numLocals)
{
    TU_ASSERT (m_code.bytecodeSize() > 0);
}

/**
 *
 * @param state
 */
lyric_assembler::ProcHandle::ProcHandle(
    const lyric_common::SymbolUrl &activation,
    ObjectState *state)
    : m_activation(activation),
      m_numListParameters(0),
      m_numNamedParameters(0),
      m_hasRestParameter(false),
      m_numLocals(0)
{
    TU_ASSERT (state != nullptr);
    m_block = new BlockHandle(this, &m_code, state, true);
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
      m_numLocals(0)
{
    TU_ASSERT (state != nullptr);
    TU_ASSERT (parent != nullptr);
    m_block = new BlockHandle(initialBindings, this, &m_code, parent, state);
}

lyric_assembler::ProcHandle::~ProcHandle()
{
    delete m_block;
}

lyric_assembler::BlockHandle *
lyric_assembler::ProcHandle::procBlock()
{
    return m_block;
}

lyric_assembler::CodeBuilder *
lyric_assembler::ProcHandle::procCode()
{
    return &m_code;
}

lyric_common::SymbolUrl
lyric_assembler::ProcHandle::getActivation() const
{
    return m_activation;
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
    CallAddress activationCall)
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
