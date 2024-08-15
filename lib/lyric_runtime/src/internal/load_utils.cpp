
#include <lyric_runtime/internal/load_utils.h>
#include <lyric_runtime/internal/resolve_link.h>

tempo_utils::Status
lyric_runtime::internal::push_literal_onto_stack(
    const BytecodeSegment *sp,
    tu_uint32 address,
    StackfulCoroutine *currentCoro,
    SegmentManagerData *segmentManagerData)
{
    InterpreterStatus status;
    auto literal = resolve_literal(sp, address, segmentManagerData, status);
    if (!literal.isValid())
        return status;
    currentCoro->pushData(DataCell::forLiteral(literal));
    return InterpreterStatus::ok();
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
    auto *segment = get_or_load_segment(symbolUrl.getModuleLocation(), segmentManagerData);
    if (segment == nullptr)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kMissingObject, symbolUrl.getModuleLocation().toString());
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
