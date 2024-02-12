
#include <lyric_runtime/internal/assembly_reader.h>
#include <lyric_runtime/internal/load_utils.h>
#include <lyric_runtime/internal/resolve_link.h>

tempo_utils::Status
lyric_runtime::internal::push_literal_onto_stack(
    tu_uint32 address,
    const BytecodeSegment **ptr,
    StackfulCoroutine *currentCoro,
    SegmentManagerData *segmentManagerData)
{
    TU_ASSERT (currentCoro != nullptr);
    TU_ASSERT (segmentManagerData != nullptr);

    lyric_object::ObjectWalker object;
    tu_uint32 index;
    InterpreterStatus status;

    const BytecodeSegment *sp;
    if (ptr != nullptr) {
        sp = *ptr;
    } else {
        sp = currentCoro->peekSP();
    }

    if (lyric_object::IS_NEAR(address)) {
        object = sp->getObject().getObject();
        index = lyric_object::GET_DESCRIPTOR_OFFSET(address);
    } else {
        auto link = sp->getObject().getObject().getLink(lyric_object::GET_LINK_OFFSET(address));
        const auto *linkage = resolve_link(sp, link, segmentManagerData, status);
        if (linkage == nullptr)
            return status;
        if (linkage->linkage != lyric_object::LinkageSection::Literal)
            return InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid linkage for literal");
        auto *segment = segmentManagerData->segments[linkage->assembly];
        object = segment->getObject().getObject();
        index = linkage->value;
        if (ptr != nullptr) {
            *ptr = segment;
        }
    }

    auto literal = object.getLiteral(index);
    if (!literal.isValid())
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing literal");

    switch (literal.getValueType()) {
        case lyric_object::ValueType::Nil:
            currentCoro->pushData(DataCell::nil());
            return InterpreterStatus::ok();

        case lyric_object::ValueType::Bool:
            currentCoro->pushData(DataCell(literal.boolValue()));
            return InterpreterStatus::ok();

        case lyric_object::ValueType::Int64:
            currentCoro->pushData(DataCell(literal.int64Value()));
            return InterpreterStatus::ok();

        case lyric_object::ValueType::Float64:
            currentCoro->pushData(DataCell(literal.float64Value()));
            return InterpreterStatus::ok();

        case lyric_object::ValueType::Char:
            currentCoro->pushData(DataCell(literal.charValue()));
            return InterpreterStatus::ok();

        case lyric_object::ValueType::String: {
            auto str = literal.stringValue();
            currentCoro->pushData(DataCell::forUtf8(str.data(), str.size()));
            return InterpreterStatus::ok();
        }

        default:
            return InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid literal type");
    }

    TU_UNREACHABLE();
}

tempo_utils::Status
lyric_runtime::internal::push_descriptor_onto_stack(
    const lyric_runtime::BytecodeSegment *sp,
    tu_uint8 section,
    tu_uint32 address,
    StackfulCoroutine *currentCoro,
    SegmentManagerData *segmentManagerData)
{
    InterpreterStatus status;
    auto linkage = lyric_object::descriptor_to_linkage_section(section);
    auto descriptor = resolve_descriptor(sp, linkage, address, segmentManagerData, status);
    if (!descriptor.isValid())
        return status;
    currentCoro->pushData(descriptor);
    return InterpreterStatus::ok();
}

tempo_utils::Status
lyric_runtime::internal::push_symbol_descriptor_onto_stack(
    const lyric_common::SymbolUrl &symbolUrl,
    StackfulCoroutine *currentCoro,
    SegmentManagerData *segmentManagerData)
{
    auto *segment = load_assembly(symbolUrl.getAssemblyLocation(), segmentManagerData);
    if (segment == nullptr)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kMissingAssembly, symbolUrl.getAssemblyLocation().toString());
    auto object = segment->getObject().getObject();
    auto symbol = object.findSymbol(symbolUrl.getSymbolPath());
    if (!symbol.isValid())
        return InterpreterStatus::forCondition(
            InterpreterCondition::kMissingSymbol, symbolUrl.getSymbolPath().toString());

    InterpreterStatus status;
    auto descriptor = resolve_descriptor(segment, symbol.getLinkageSection(), symbol.getLinkageIndex(),
        segmentManagerData, status);
    if (!descriptor.isValid())
        return status;
    currentCoro->pushData(descriptor);
    return InterpreterStatus::ok();
}
