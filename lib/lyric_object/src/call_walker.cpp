
#include <lyric_object/call_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/parameter_walker.h>
#include <lyric_object/symbol_walker.h>
#include <tempo_utils/big_endian.h>

lyric_object::CallWalker::CallWalker()
    : m_callOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::CallWalker::CallWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 callOffset)
    : m_reader(reader),
      m_callOffset(callOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_callOffset != INVALID_ADDRESS_U32);
}

lyric_object::CallWalker::CallWalker(const CallWalker &other)
    : m_reader(other.m_reader),
      m_callOffset(other.m_callOffset)
{
}

bool
lyric_object::CallWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_callOffset < m_reader->numCalls();
}

lyric_common::SymbolPath
lyric_object::CallWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return {};
    if (callDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(callDescriptor->fqsn()->str());
}

bool
lyric_object::CallWalker::isConstructor() const
{
    if (!isValid())
        return false;
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return false;
    return bool(callDescriptor->flags() & lyo1::CallFlags::Ctor);
}

bool
lyric_object::CallWalker::isBound() const
{
    if (!isValid())
        return false;
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return false;
    return bool(callDescriptor->flags() & lyo1::CallFlags::Bound);
}

bool
lyric_object::CallWalker::isNoReturn() const
{
    if (!isValid())
        return false;
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return false;
    return bool(callDescriptor->flags() & lyo1::CallFlags::NoReturn);
}

bool
lyric_object::CallWalker::isDeclOnly() const
{
    if (!isValid())
        return false;
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return false;
    return bool(callDescriptor->flags() & lyo1::CallFlags::DeclOnly);
}

lyric_object::AccessType
lyric_object::CallWalker::getAccess() const
{
    if (!isValid())
        return AccessType::Invalid;
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return AccessType::Invalid;

    if (bool(callDescriptor->flags() & lyo1::CallFlags::GlobalVisibility))
        return AccessType::Public;
    if (bool(callDescriptor->flags() & lyo1::CallFlags::InheritVisibility))
        return AccessType::Protected;
    return AccessType::Private;
}

lyric_object::CallMode
lyric_object::CallWalker::getMode() const
{
    if (!isValid())
        return CallMode::Invalid;
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return CallMode::Invalid;

    if (bool(callDescriptor->flags() & lyo1::CallFlags::Ctor))
        return CallMode::Constructor;
    if (bool(callDescriptor->flags() & lyo1::CallFlags::Inline))
        return CallMode::Inline;
    return CallMode::Normal;
}

bool
lyric_object::CallWalker::hasTemplate() const
{
    if (!isValid())
        return false;
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return false;
    return callDescriptor->call_template() != INVALID_ADDRESS_U32;
}

lyric_object::TemplateWalker
lyric_object::CallWalker::getTemplate() const
{
    if (!hasTemplate())
        return {};
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return {};

    return TemplateWalker(m_reader, callDescriptor->call_template());
}

bool
lyric_object::CallWalker::hasReceiver() const
{
    if (!isValid())
        return {};
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return {};
    return callDescriptor->receiver_section() != lyo1::TypeSection::Invalid
        && callDescriptor->receiver_descriptor() != INVALID_ADDRESS_U32;
}

lyric_object::SymbolWalker
lyric_object::CallWalker::getReceiver() const
{
    if (!hasReceiver())
        return {};
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return {};
    lyo1::DescriptorSection section;
    switch (callDescriptor->receiver_section()) {
        case lyo1::TypeSection::Class:
            section = lyo1::DescriptorSection::Class;
            break;
        case lyo1::TypeSection::Concept:
            section = lyo1::DescriptorSection::Concept;
            break;
        case lyo1::TypeSection::Enum:
            section = lyo1::DescriptorSection::Enum;
            break;
        case lyo1::TypeSection::Existential:
            section = lyo1::DescriptorSection::Existential;
            break;
        case lyo1::TypeSection::Instance:
            section = lyo1::DescriptorSection::Instance;
            break;
        case lyo1::TypeSection::Struct:
            section = lyo1::DescriptorSection::Struct;
            break;
        case lyo1::TypeSection::Type:
            section = lyo1::DescriptorSection::Type;
            break;
        default:
            return {};
    }
    tu_uint32 index = callDescriptor->receiver_descriptor();
    auto symbolPath = m_reader->getSymbolPath(section, index);
    return SymbolWalker(m_reader, m_reader->getSymbolIndex(symbolPath));
}

tu_uint8
lyric_object::CallWalker::numListParameters() const
{
    if (!isValid())
        return 0;
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return 0;
    if (callDescriptor->list_parameters() == nullptr)
        return 0;
    return callDescriptor->list_parameters()->size();
}

lyric_object::ParameterWalker
lyric_object::CallWalker::getListParameter(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return {};
    if (callDescriptor->list_parameters() == nullptr)
        return {};
    auto *listParameters = callDescriptor->list_parameters();

    if (listParameters->size() <= index)
        return {};
    auto *parameter = listParameters->Get(index);
    auto placement = parameter->initializer_call() == INVALID_ADDRESS_U32? PlacementType::List
        : PlacementType::ListOpt;

    return ParameterWalker(m_reader, (void *) parameter, index, placement);
}

tu_uint8
lyric_object::CallWalker::numNamedParameters() const
{
    if (!isValid())
        return false;
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return {};
    if (callDescriptor->named_parameters() == nullptr)
        return 0;
    return callDescriptor->named_parameters()->size();
}

lyric_object::ParameterWalker
lyric_object::CallWalker::getNamedParameter(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return {};
    if (callDescriptor->named_parameters() == nullptr)
        return {};
    auto *namedParameters = callDescriptor->named_parameters();

    if (namedParameters->size() <= index)
        return {};
    auto *parameter = namedParameters->Get(index);

    PlacementType placement = PlacementType::Invalid;
    if (bool(parameter->flags() & lyo1::ParameterFlags::Ctx)) {
        placement = PlacementType::Ctx;
    } else if (parameter->initializer_call() != INVALID_ADDRESS_U32) {
        placement = PlacementType::NamedOpt;
    } else {
        placement = PlacementType::Named;
    }

    return ParameterWalker(m_reader, (void *) parameter, index, placement);
}

bool
lyric_object::CallWalker::hasRestParameter() const
{
    if (!isValid())
        return false;
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return false;
    return callDescriptor->rest_parameter() != nullptr;
}

lyric_object::ParameterWalker
lyric_object::CallWalker::getRestParameter() const
{
    if (!isValid())
        return {};
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return {};
    auto *parameter =  callDescriptor->rest_parameter();
    if (parameter == nullptr)
        return {};
    return ParameterWalker(m_reader, (void *) parameter, 0, PlacementType::Rest);
}

inline void
read_proc_header(const tu_uint8 * const header, lyric_object::ProcHeader &procHeader)
{
    const tu_uint8 *bytes = header;
    procHeader.procSize = tempo_utils::read_u32_and_advance(bytes);
    procHeader.numArguments = tempo_utils::read_u16_and_advance(bytes);
    procHeader.numLocals = tempo_utils::read_u16_and_advance(bytes);
    procHeader.numLexicals = tempo_utils::read_u16_and_advance(bytes);
    procHeader.headerSize = 10 + (procHeader.numLexicals * 9);
}

std::span<const tu_uint8>
lyric_object::CallWalker::getProc() const
{
    if (!isValid())
        return {};
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return {};

    std::span<const tu_uint8> bytecode(m_reader->getBytecodeData(), m_reader->getBytecodeSize());

    // verify that proc offset is within the segment bytecode
    auto procOffset = callDescriptor->bytecode_offset();
    if (bytecode.size() <= procOffset)
        return {};

    // verify that proc is large enough to fit header
    if (bytecode.size() - procOffset < 10)
        return {};

    auto *start = bytecode.data() + procOffset;

    // read the call proc header
    ProcHeader procHeader;
    read_proc_header(start, procHeader);

    // return a span containing the header and bytecode
    return std::span<const tu_uint8>(start, procHeader.procSize + 4);
}

lyric_object::ProcHeader
lyric_object::CallWalker::getProcHeader() const
{
    if (!isValid())
        return {};
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return {};

    std::span<const tu_uint8> bytecode(m_reader->getBytecodeData(), m_reader->getBytecodeSize());

    // verify that proc offset is within the segment bytecode
    auto procOffset = callDescriptor->bytecode_offset();
    if (bytecode.size() <= procOffset)
        return {};

    // verify that proc is large enough to fit header
    if (bytecode.size() - procOffset < 10)
        return {};

    auto *start = bytecode.data() + procOffset;

    // read the call proc header
    ProcHeader procHeader;
    read_proc_header(start, procHeader);
    return procHeader;
}

tu_uint32
lyric_object::CallWalker::getProcOffset() const
{
    if (!isValid())
        return {};
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return {};
    return callDescriptor->bytecode_offset();
}

lyric_object::BytecodeIterator
lyric_object::CallWalker::getBytecodeIterator() const
{
    if (!isValid())
        return {};

    auto procHeader = getProcHeader();
    if (procHeader.procSize == 0)
        return {};

    auto proc = getProc();
    if (proc.empty())
        return {};
    return BytecodeIterator(proc.data() + procHeader.headerSize, proc.size() - procHeader.headerSize);
}

lyric_object::TypeWalker
lyric_object::CallWalker::getResultType() const
{
    if (!isValid())
        return {};
    auto *callDescriptor = m_reader->getCall(m_callOffset);
    if (callDescriptor == nullptr)
        return {};
    return TypeWalker(m_reader, callDescriptor->result_type());
}

tu_uint32
lyric_object::CallWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_callOffset;
}
