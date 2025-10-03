#include <absl/strings/substitute.h>

#include <lyric_object/bytecode_iterator.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

#include "closure_ref.h"

ClosureRef::ClosureRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_segmentIndex(lyric_runtime::INVALID_ADDRESS_U32),
      m_callIndex(lyric_runtime::INVALID_ADDRESS_U32),
      m_procOffset(lyric_runtime::INVALID_ADDRESS_U32),
      m_lexicals()
{
}

ClosureRef::~ClosureRef()
{
    TU_LOG_VV << "free" << ClosureRef::toString();
    m_lexicals.clear();
}

lyric_runtime::DataCell
ClosureRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
ClosureRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

bool
ClosureRef::applyClosure(
    lyric_runtime::Task *task,
    std::vector<lyric_runtime::DataCell> &args,
    lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = task->stackfulCoroutine();
    auto dataStackGuard = currentCoro->dataStackSize();

    auto *sp = currentCoro->peekSP();
    tu_uint32 returnSegment = lyric_object::INVALID_ADDRESS_U32;
    lyric_object::BytecodeIterator returnIP{};

    if (sp != nullptr) {
        returnSegment = sp->getSegmentIndex();
        returnIP = currentCoro->peekIP();
    }


    auto *segment = state->segmentManager()->getSegment(getSegmentIndex());
    TU_ASSERT (segment != nullptr);
    auto address = getCallIndex();
    auto procOffset = getProcOffset();
    auto returnsValue = this->returnsValue();

    // proc offset must be within the segment bytecode
    auto *bytecodeData = segment->getBytecodeData();
    auto bytecodeSize = segment->getBytecodeSize();
    if (bytecodeSize <= procOffset)
        throw tempo_utils::StatusException(
            lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid proc offset"));

    const tu_uint8 *header = bytecodeData + procOffset;

    header += 4;                                                        // skip over procSize
    auto numArguments = tempo_utils::read_u16_and_advance(header);      // read numArguments
    auto numLocals = tempo_utils::read_u16_and_advance(header);         // read numLocals
    auto numLexicals = tempo_utils::read_u16_and_advance(header);       // read numLexicals

    // maximum number of args is 2^16
    if (numArguments != args.size())
        throw tempo_utils::StatusException(
            lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "too many arguments"));

    // construct the task activation call frame
    lyric_runtime::CallCell frame(address, segment->getSegmentIndex(), procOffset, returnSegment,
        returnIP, returnsValue, dataStackGuard, numArguments, /* numRest= */ 0, numLocals, numLexicals, args);

    if (this->numLexicals() != numLexicals)
        throw tempo_utils::StatusException(
            lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "not enough arguments"));

    // import each lexical from the instance into the stack
    for (tu_uint16 i = 0; i < numLexicals; i++) {
        frame.setLexical(i, lexicalAt(i));
    }

    // push the lambda onto the call stack
    auto ip = getIP();
    currentCoro->pushCall(frame, ip, segment);
    TU_LOG_VV << "moved ip to " << ip;

    return true;
}

std::string
ClosureRef::toString() const
{
    return absl::Substitute("<$0: ClosureRef capturing call $1 in segment $2>",
        this, m_callIndex, m_segmentIndex);
}

tu_uint32
ClosureRef::getSegmentIndex() const
{
    return m_segmentIndex;
}

void
ClosureRef::setSegmentIndex(tu_uint32 segmentIndex)
{
    m_segmentIndex = segmentIndex;
}

tu_uint32
ClosureRef::getCallIndex() const
{
    return m_callIndex;
}

void
ClosureRef::setCallIndex(tu_uint32 callIndex)
{
    m_callIndex = callIndex;
}

tu_uint32
ClosureRef::getProcOffset() const
{
    return m_procOffset;
}

void
ClosureRef::setProcOffset(tu_uint32 procOffset)
{
    m_procOffset = procOffset;
}

bool
ClosureRef::returnsValue() const
{
    return m_returnsValue;
}

void
ClosureRef::setReturnsValue(bool returnsValue)
{
    m_returnsValue = returnsValue;
}

lyric_object::BytecodeIterator
ClosureRef::getIP() const
{
    return m_IP;
}

void
ClosureRef::setIP(lyric_object::BytecodeIterator ip)
{
    m_IP = ip;
}

lyric_runtime::DataCell
ClosureRef::lexicalAt(int index) const
{
    if (0 <= index && std::cmp_less(index, m_lexicals.size()))
        return m_lexicals[index];
    return lyric_runtime::DataCell();
}

void
ClosureRef::lexicalAppend(const lyric_runtime::DataCell &value)
{
    m_lexicals.push_back(value);
}

int
ClosureRef::numLexicals() const
{
    return m_lexicals.size();
}

void
ClosureRef::setMembersReachable()
{
    for (auto &cell : m_lexicals) {
        if (cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->setReachable();
        }
    }
}

void
ClosureRef::clearMembersReachable()
{
    for (auto &cell : m_lexicals) {
        if (cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->clearReachable();
        }
    }
}

tempo_utils::Status
closure_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();

    auto ref = state->heapManager()->allocateRef<ClosureRef>(vtable);
    currentCoro->pushData(ref);

    return {};
}

tempo_utils::Status
closure_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::CALL);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<ClosureRef *>(receiver.data.ref);
    instance->setSegmentIndex(arg0.data.descriptor->getSegmentIndex());
    instance->setCallIndex(arg0.data.descriptor->getDescriptorIndex());

    // fetch the proc offset for the call descriptor
    auto *segment = state->segmentManager()->getSegment(arg0.data.descriptor->getSegmentIndex());
    TU_ASSERT (segment != nullptr);
    auto object = segment->getObject();
    auto descriptor = object.getCall(arg0.data.descriptor->getDescriptorIndex());
    auto procOffset = descriptor.getProcOffset();

    // parse the proc
    auto bytecode = segment->getBytecode();
    lyric_object::ProcInfo procInfo;
    TU_RETURN_IF_NOT_OK (lyric_object::parse_proc_info(bytecode, procOffset, procInfo));

    // set the proc offset
    instance->setProcOffset(procOffset);

    // set returnsValue flag
    instance->setReturnsValue(!descriptor.isNoReturn());

    std::vector<lyric_object::ProcLexical> lexicals;
    TU_RETURN_IF_NOT_OK (lyric_object::parse_lexicals_table(procInfo, lexicals));

    // import each lexical from the latest activation and add it to the closure instance
    for (int i = 0; i < lexicals.size(); i++) {
        const auto &lexical = lexicals.at(i);

        // starting from the top of the stack, work our way down until we find the activation
        bool found = false;
        for (auto iterator = currentCoro->callsBegin(); iterator != currentCoro->callsEnd(); iterator++) {
            const auto &ancestor = *iterator;

            if (ancestor.getCallSegment() != segment->getSegmentIndex())    // frame does not match the segment index
                continue;
            if (ancestor.getCallIndex() != lexical.activation_call)         // frame does not match activation call
                continue;
            switch (lexical.lexical_target) {
                case lyric_object::LEXICAL_ARGUMENT:
                    instance->lexicalAppend(ancestor.getArgument(lexical.target_offset));
                    break;
                case lyric_object::LEXICAL_LOCAL:
                    instance->lexicalAppend(ancestor.getLocal(lexical.target_offset));
                    break;
                default:
                    return lyric_runtime::InterpreterStatus::forCondition(
                        lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid lexical target");
            }
            found = true;   // we found the lexical, break loop early
            break;
        }
        if (!found)
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "missing lexical");
    }

    // push the lambda onto the call stack
    lyric_object::BytecodeIterator ip(procInfo.code);
    instance->setIP(ip);

    return {};
}

tempo_utils::Status
closure_apply(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    // pop the enclosing frame off the call stack
    lyric_runtime::CallCell frame;
    TU_RETURN_IF_NOT_OK (currentCoro->popCall(frame));

    // pop any temporaries related to the previous frame off the data stack
    currentCoro->resizeDataStack(frame.getStackGuard());

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<ClosureRef *>(receiver.data.ref);

    auto *segment = state->segmentManager()->getSegment(instance->getSegmentIndex());
    TU_ASSERT (segment != nullptr);
    auto address = instance->getCallIndex();
    auto procOffset = instance->getProcOffset();
    auto returnsValue = instance->returnsValue();

    // proc offset must be within the segment bytecode
    auto *bytecodeData = segment->getBytecodeData();
    auto bytecodeSize = segment->getBytecodeSize();
    if (bytecodeSize <= procOffset)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid proc offset");

    const tu_uint8 *header = bytecodeData + procOffset;
    auto returnSP = frame.getReturnSegment();
    auto returnIP = frame.getReturnIP();
    auto stackGuard = currentCoro->dataStackSize();

    header += 4;                                                        // skip over procSize
    auto numArguments = tempo_utils::read_u16_and_advance(header);      // read numArguments
    auto numLocals = tempo_utils::read_u16_and_advance(header);         // read numLocals
    auto numLexicals = tempo_utils::read_u16_and_advance(header);       // read numLexicals

    // maximum number of args is 2^16
    if (std::numeric_limits<tu_uint16>::max() <= frame.numArguments())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "too many arguments");
    // all required args must be present
    if (frame.numArguments() < numArguments)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "not enough arguments");
    auto numRest = frame.numRest();

    // copy the args (including rest args) from the enclosing frame
    std::vector<lyric_runtime::DataCell> args(frame.numArguments() + frame.numRest());
    for (int i = 0; i < frame.numArguments(); i++) {
        args[i] = frame.getArgument(i);
    }
    for (int i = 0; i < frame.numRest(); i++) {
        args[numArguments + i] = frame.getRest(i);
    }

    // construct the lambda activation call frame
    lyric_runtime::CallCell trampoline(address, segment->getSegmentIndex(),
        procOffset, returnSP, returnIP, returnsValue, stackGuard, numArguments,
        numRest, numLocals, numLexicals, args, receiver);

    if (instance->numLexicals() != numLexicals)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "not enough arguments");

    // import each lexical from the instance into the stack
    for (tu_uint16 i = 0; i < numLexicals; i++) {
        trampoline.setLexical(i, instance->lexicalAt(i));
    }

    // push the lambda onto the call stack
    auto ip = instance->getIP();
    currentCoro->pushCall(trampoline, ip, segment);
    TU_LOG_VV << "moved ip to " << ip;

    return {};
}
