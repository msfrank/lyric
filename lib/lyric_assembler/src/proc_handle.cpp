
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/check_handle.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <tempo_utils/bytes_appender.h>
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
      m_state(state),
      m_numLocals(0),
      m_nextId(0)
{
    TU_ASSERT (state != nullptr);
    m_fragment = std::unique_ptr<CodeFragment>(new CodeFragment(this));
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
      m_state(state),
      m_numLocals(0),
      m_nextId(0)
{
    TU_ASSERT (state != nullptr);
    TU_ASSERT (parent != nullptr);
    absl::flat_hash_map<std::string,SymbolBinding> initialBindings;
    m_block = std::make_unique<BlockHandle>(initialBindings, this, parent, state);
    m_fragment = std::unique_ptr<CodeFragment>(new CodeFragment(this));
}

/**
 * Allocate a new ProcHandle for a normal call.
 *
 * @param activation
 * @param initialBindings
 * @param numListParameters
 * @param numNamedParameters
 * @param hasRestParameter
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
      m_state(state),
      m_numLocals(0),
      m_nextId(0)
{
    TU_ASSERT (state != nullptr);
    TU_ASSERT (parent != nullptr);
    m_block = std::make_unique<BlockHandle>(initialBindings, this, parent, state);
    m_fragment = std::unique_ptr<CodeFragment>(new CodeFragment(this));
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

lyric_assembler::CodeFragment *
lyric_assembler::ProcHandle::procFragment()
{
    return m_fragment.get();
}

const lyric_assembler::CodeFragment *
lyric_assembler::ProcHandle::procFragment() const
{
    return m_fragment.get();
}

lyric_assembler::ObjectState *
lyric_assembler::ProcHandle::objectState() const
{
    return m_state;
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
    auto checkHandle = std::make_unique<CheckHandle>(checkStart, this, m_state);
    auto *checkPtr = checkHandle.get();
    m_checks.push_back(std::move(checkHandle));
    return checkPtr;
}

int
lyric_assembler::ProcHandle::numChecks() const
{
    return m_checks.size();
}

tempo_utils::Result<lyric_assembler::CatchHandle *>
lyric_assembler::ProcHandle::declareCatch(const lyric_common::TypeDef &exceptionType)
{
    auto fragment = m_fragment->makeFragment();
    auto catchHandle = std::make_unique<CatchHandle>(exceptionType, fragment.get(), m_state);
    auto *catchPtr = catchHandle.get();
    std::pair entry(std::move(fragment), std::move(catchHandle));
    m_catches.push_back(std::move(entry));
    return catchPtr;
}

int
lyric_assembler::ProcHandle::numCatches() const
{
    return m_catches.size();
}

tempo_utils::Result<lyric_assembler::CleanupHandle *>
lyric_assembler::ProcHandle::declareCleanup(const JumpLabel &cleanupStart)
{
    auto cleanupHandle = std::make_unique<CleanupHandle>(cleanupStart, m_state);
    auto *cleanupPtr = cleanupHandle.get();
    m_cleanups.push_back(std::move(cleanupHandle));
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

absl::flat_hash_set<tu_uint32>
lyric_assembler::ProcHandle::getTargetsForLabel(std::string_view labelName) const
{
    auto entry = m_labelTargets.find(labelName);
    if (entry != m_labelTargets.cend())
        return entry->second;
    return {};
}

std::string
lyric_assembler::ProcHandle::getLabelForTarget(tu_uint32 targetId) const
{
    auto entry = m_jumpLabels.find(targetId);
    if (entry != m_jumpLabels.cend())
        return entry->second;
    return {};
}

tempo_utils::Result<std::string>
lyric_assembler::ProcHandle::makeLabel(std::string_view userLabel)
{
    std::string labelName(userLabel);
    if (labelName.empty()) {
        if (m_nextId == std::numeric_limits<tu_uint32>::max())
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "too many ids");
        labelName = absl::StrCat("label", m_nextId);
        if (m_labelTargets.contains(labelName))
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "assembly label {} already exists", labelName);
        m_nextId++;
    } else {
        if (m_labelTargets.contains(labelName))
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "assembly label {} already exists", labelName);
    }

    m_labelTargets[labelName] = {};
    TU_LOG_VV << "proc " << this << " makes label " << labelName;
    return labelName;
}

tempo_utils::Result<tu_uint32>
lyric_assembler::ProcHandle::makeJump()
{
    if (m_nextId == std::numeric_limits<tu_uint32>::max())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "too many ids");

    auto targetId = m_nextId++;
    m_jumpLabels[targetId] = {};
    TU_LOG_VV << "proc " << this << " makes jump " << targetId;
    return targetId;
}

tempo_utils::Status
lyric_assembler::ProcHandle::patchTarget(tu_uint32 targetId, std::string_view labelName)
{
    TU_ASSERT (!labelName.empty());

    auto labelTargetEntry = m_labelTargets.find(labelName);
    if (labelTargetEntry == m_labelTargets.cend())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "missing assembly label {}", labelName);
    auto &labelTargetSet = labelTargetEntry->second;

    auto jumpLabelEntry = m_jumpLabels.find(targetId);
    if (jumpLabelEntry == m_jumpLabels.cend())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "missing jump target");
    auto &jumpLabel = jumpLabelEntry->second;

    labelTargetSet.insert(targetId);
    jumpLabel = labelName;

    TU_LOG_VV << "proc " << this << " patches target " << targetId << " to label " << std::string(labelName);
    return {};
}

tempo_utils::Status
lyric_assembler::ProcHandle::touch(ObjectWriter &writer) const
{
    TU_RETURN_IF_NOT_OK (m_fragment->touch(writer));
    for (const auto &entry : m_catches) {
        auto &catchHandle = entry.second;
        auto *catchFragment = catchHandle->catchFragment();
        TU_RETURN_IF_NOT_OK (catchFragment->touch(writer));
    }
    return {};
}

inline tempo_utils::Status
validate_fragment(const std::unique_ptr<lyric_assembler::CodeFragment> &fragment)
{
    TU_ASSERT (fragment != nullptr);

    auto numStatements = fragment->numStatements();

    // verify the fragment is not empty
    if (numStatements == 0)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kAssemblerInvariant,
            "invalid empty proc");

    // verify that the last statement unconditionally transfers control or terminates
    auto lastStatement = fragment->getStatement(fragment->numStatements() - 1);
    switch (lastStatement.instruction->getType()) {
        case lyric_assembler::InstructionType::Jump:
        case lyric_assembler::InstructionType::Return:
        case lyric_assembler::InstructionType::Interrupt:
        case lyric_assembler::InstructionType::Abort:
        case lyric_assembler::InstructionType::Halt:
            return {};
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                "invalid proc; final instruction must transfer control or terminate");
    }
}

tempo_utils::Status
lyric_assembler::ProcHandle::build(
    const ObjectWriter &writer,
    tempo_utils::BytesAppender &procBytecode) const
{
    lyric_object::BytecodeBuilder bytecodeBuilder;
    absl::flat_hash_map<std::string,tu_uint16> labelOffsets;
    absl::flat_hash_map<tu_uint32,tu_uint16> patchOffsets;

    // ensure fragment is valid
    TU_RETURN_IF_NOT_OK (validate_fragment(m_fragment));

    // write the main bytecode
    TU_RETURN_IF_NOT_OK (m_fragment->build(writer, bytecodeBuilder, labelOffsets, patchOffsets));

    std::vector<lyric_object::ProcCheck> checks;
    std::vector<lyric_object::ProcException> exceptions;
    std::vector<lyric_object::ProcCleanup> cleanups;

    // write each catch
    for (const auto &entry : m_catches) {
        auto &catchFragment = entry.first;
        auto &catchHandle = entry.second;
        TU_RETURN_IF_NOT_OK (validate_fragment(catchFragment));
        lyric_object::ProcException exception;
        TU_ASSIGN_OR_RETURN (exception.exception_type, writer.getTypeOffset(catchHandle->getExceptionType()));
        TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_ABORT));
        exception.catch_offset = bytecodeBuilder.bytecodeSize();
        TU_RETURN_IF_NOT_OK (catchFragment->build(writer, bytecodeBuilder, labelOffsets, patchOffsets));
        auto endExclusive = bytecodeBuilder.bytecodeSize();
        exception.catch_size = endExclusive - exception.catch_offset;
        exceptions.push_back(exception);
    }

    TU_LOG_VV << "label targets:";
    for (const auto &entry : m_labelTargets) {
        TU_LOG_VV << "  label " << entry.first;
        for (const auto &target : entry.second) {
            TU_LOG_VV << "    target " << target;
        }
    }
    TU_LOG_VV << "jump labels:";
    for (const auto &entry : m_jumpLabels) {
        TU_LOG_VV << "  target " << entry.first << " -> " << entry.second;
    }

    // patch jump targets
    for (const auto &entry : patchOffsets) {
        auto targetId = entry.first;
        auto patchOffset = entry.second;
        const auto &jumpLabel = m_jumpLabels.at(targetId);
        auto labelOffset = labelOffsets.find(jumpLabel);
        if (labelOffset == labelOffsets.cend())
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "missing label '{}' for patch target {}", jumpLabel, targetId);
        TU_RETURN_IF_NOT_OK (bytecodeBuilder.patch(patchOffset, labelOffset->second));
    }

    // build the trailer if needed
    tempo_utils::BytesAppender trailer;

    if (!checks.empty() || !exceptions.empty() || !cleanups.empty()) {
        trailer.appendU16(static_cast<tu_uint16>(checks.size()));
        trailer.appendU16(static_cast<tu_uint16>(exceptions.size()));
        trailer.appendU16(static_cast<tu_uint16>(cleanups.size()));

        for (const auto &check : checks) {
            trailer.appendU32(check.interval_offset);
            trailer.appendU32(check.interval_size);
            trailer.appendU16(check.parent_check);
            trailer.appendU16(check.first_exception);
            trailer.appendU16(check.num_exceptions);
        }

        for (const auto &exception : exceptions) {
            trailer.appendU32(exception.exception_type);
            trailer.appendU32(exception.catch_offset);
            trailer.appendU32(exception.catch_size);
        }

        for (const auto &cleanup : cleanups) {
            trailer.appendU32(cleanup.interval_offset);
            trailer.appendU32(cleanup.interval_size);
            trailer.appendU16(cleanup.parent_cleanup);
            trailer.appendU32(cleanup.cleanup_offset);
            trailer.appendU32(cleanup.cleanup_size);
        }
    }

    // build the header
    tempo_utils::BytesAppender header;
    header.appendU16(static_cast<tu_uint16>(getArity()));
    header.appendU16(static_cast<tu_uint16>(numLocals()));
    header.appendU16(static_cast<tu_uint16>(numLexicals()));
    header.appendU32(trailer.getSize());

    for (const auto &lexical : m_lexicals) {
        tu_uint32 address;
        TU_ASSIGN_OR_RETURN (address, writer.getSectionAddress(
            lexical.activationCall->getSymbolUrl(), lyric_object::LinkageSection::Call));
        header.appendU32(address);
        header.appendU32(lexical.targetOffset);
        header.appendU8(static_cast<tu_uint8>(lexical.lexicalTarget));
    }

    // build the proc
    procBytecode.appendU32(header.getSize() + bytecodeBuilder.bytecodeSize() + trailer.getSize());
    procBytecode.appendBytes(header);
    procBytecode.appendBytes(std::span(bytecodeBuilder.bytecodeBegin(), bytecodeBuilder.bytecodeEnd()));
    procBytecode.appendBytes(trailer);

    return {};
}