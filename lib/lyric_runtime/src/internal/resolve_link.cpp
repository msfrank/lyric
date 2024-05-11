
#include <lyric_runtime/internal/assembly_reader.h>
#include <lyric_runtime/internal/resolve_link.h>

lyric_runtime::BytecodeSegment *
lyric_runtime::internal::load_assembly(
    const lyric_common::AssemblyLocation &location,
    SegmentManagerData *segmentManagerData)
{
    // return null if assembly location is invalid
    if (!location.isValid())
        return nullptr;

    // if assembly is already loaded then return the segment
    if (segmentManagerData->segmentcache.contains(location))
        return segmentManagerData->segments[segmentManagerData->segmentcache[location]];

    auto loadAssemblyResult = segmentManagerData->loader->loadAssembly(location);
    if (loadAssemblyResult.isStatus()) {
        TU_LOG_V << "failed to load " << location << ": " << loadAssemblyResult.getStatus();
        return nullptr;                                 // failed to load assembly from location
    }
    auto assemblyOption = loadAssemblyResult.getResult();
    if (assemblyOption.isEmpty()) {
        TU_LOG_V << "failed to load " << location << ": assembly not found";
        return nullptr;                                 // failed to load assembly from location
    }
    auto assembly = assemblyOption.getValue();

    // attempt to load the plugin for the assembly
    // FIXME: its not currently considered an error if the plugin is missing.
    auto loadPluginResult = segmentManagerData->loader->loadPlugin(
        location, lyric_object::PluginSpecifier::systemDefault());
    if (loadPluginResult.isStatus())
        return nullptr;
    auto pluginOption = loadPluginResult.getResult();
    auto plugin = pluginOption.getOrDefault({});

    auto segmentIndex = segmentManagerData->segments.size();
    auto *segment = new lyric_runtime::BytecodeSegment(segmentIndex, location, assembly, plugin);
    segmentManagerData->segments.push_back(segment);
    segmentManagerData->segmentcache[location] = segmentIndex;
    return segment;
}

const lyric_runtime::LinkEntry *
lyric_runtime::internal::resolve_link(
    const lyric_runtime::BytecodeSegment *sp,
    const lyric_object::LinkWalker &link,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    if (!link.isValid()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid address for link");
        return nullptr;
    }

    //auto index = address & 0x7FFFFFFF;
    auto index = link.getDescriptorOffset();
    auto *linkageEntry = sp->getLink(index);
    if (linkageEntry == nullptr) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing linkage");
        return nullptr;
    }

    // if linkageType is not UNKNOWN, then dynamic linking has completed, so return the linkage entry
    if (linkageEntry->linkage != lyric_object::LinkageSection::Invalid)
        return linkageEntry;

    // get the link descriptor in the segment assembly
    auto currentObject = sp->getObject().getObject();
    TU_ASSERT (currentObject.isValid());
    auto currentLink = currentObject.getLink(index);
    TU_ASSERT (currentLink.isValid());
    auto referenceUrl = currentLink.getLinkUrl();
    if (!referenceUrl.isValid()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid link url");
        return nullptr;
    }
    TU_LOG_V << "resolving link " << index << " to symbol " << referenceUrl;

    // get the segment containing the linked symbol
    auto location = referenceUrl.getAssemblyLocation();
    auto *segment = load_assembly(location, segmentManagerData);
    if (segment == nullptr) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kMissingAssembly, location.toString());
        return nullptr;
    }

    // get the symbol from the target assembly
    const auto targetObject = segment->getObject().getObject();
    TU_ASSERT (targetObject.isValid());
    auto symbolPath = referenceUrl.getSymbolPath();
    TU_LOG_V << "searching for " << symbolPath << " in " << location;
    auto symbol = targetObject.findSymbol(symbolPath);
    if (!symbol.isValid()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kMissingSymbol, symbolPath.toString());
        return nullptr;
    }

    // update the linkage entry and return pointer to the entry
    LinkEntry completedLinkage;
    completedLinkage.linkage = symbol.getLinkageSection();
    completedLinkage.value = symbol.getLinkageIndex();
    completedLinkage.assembly = segment->getSegmentIndex();

    TU_LOG_V << "resolved " << symbolPath
             << " to descriptor " << completedLinkage.value
             << " in assembly " << completedLinkage.assembly;

    segment = segmentManagerData->segments[sp->getSegmentIndex()];
    if (!segment->setLink(index, completedLinkage)) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "failed to set link");
        return nullptr;
    }

    return segment->getLink(index);
}

lyric_runtime::DataCell
lyric_runtime::internal::resolve_descriptor(
    const BytecodeSegment *sp,
    lyric_object::LinkageSection section,
    tu_uint32 address,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    TU_ASSERT (sp != nullptr);

    tu_uint32 segmentIndex;
    tu_uint32 valueIndex;

    if (lyric_object::IS_NEAR(address)) {
        segmentIndex = sp->getSegmentIndex();
        valueIndex = lyric_object::GET_DESCRIPTOR_OFFSET(address);
    } else {
        auto link = sp->getObject().getObject().getLink(lyric_object::GET_LINK_OFFSET(address));
        const auto *linkage = resolve_link(sp, link, segmentManagerData, status);
        if (linkage == nullptr)
            return {};          // failed to resolve dynamic link
        if (linkage->linkage != section) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid linkage for descriptor");
            return {};          // wrong descriptor type
        }
        segmentIndex = linkage->assembly;
        valueIndex = linkage->value;
    }

    switch (section) {
        case lyric_object::LinkageSection::Existential:
            return DataCell::forExistential(segmentIndex, valueIndex);
        case lyric_object::LinkageSection::Class:
            return DataCell::forClass(segmentIndex, valueIndex);
        case lyric_object::LinkageSection::Field:
            return DataCell::forField(segmentIndex, valueIndex);
        case lyric_object::LinkageSection::Call:
            return DataCell::forCall(segmentIndex, valueIndex);
        case lyric_object::LinkageSection::Action:
            return DataCell::forAction(segmentIndex, valueIndex);
        case lyric_object::LinkageSection::Concept:
            return DataCell::forConcept(segmentIndex, valueIndex);
        case lyric_object::LinkageSection::Enum:
            return DataCell::forEnum(segmentIndex, valueIndex);
        case lyric_object::LinkageSection::Instance:
            return DataCell::forInstance(segmentIndex, valueIndex);
        case lyric_object::LinkageSection::Struct:
            return DataCell::forStruct(segmentIndex, valueIndex);
        case lyric_object::LinkageSection::Type:
            return DataCell::forType(segmentIndex, valueIndex);
        default:
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "unknown descriptor type");
            return {};
    }
}

lyric_runtime::LiteralCell
lyric_runtime::internal::resolve_literal(
    const BytecodeSegment *sp,
    tu_uint32 address,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    TU_ASSERT (sp != nullptr);

    lyric_object::ObjectWalker object;
    tu_uint32 index;

    if (lyric_object::IS_NEAR(address)) {
        object = sp->getObject().getObject();
        index = lyric_object::GET_DESCRIPTOR_OFFSET(address);
    } else {
        auto link = sp->getObject().getObject().getLink(lyric_object::GET_LINK_OFFSET(address));
        const auto *linkage = resolve_link(sp, link, segmentManagerData, status);
        if (linkage == nullptr)
            return {};          // failed to resolve dynamic link
        if (linkage->linkage != lyric_object::LinkageSection::Literal) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid linkage for literal");
            return {};          // wrong descriptor type
        }
        auto *segment = segmentManagerData->segments[linkage->assembly];
        object = segment->getObject().getObject();
        index = linkage->value;
    }

    auto literal = object.getLiteral(index);
    if (!literal.isValid()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing literal");
        return {};
    }

    switch (literal.getValueType()) {
        case lyric_object::ValueType::Nil:
            return LiteralCell::nil();
        case lyric_object::ValueType::Bool:
            return LiteralCell(literal.boolValue());
        case lyric_object::ValueType::Int64:
            return LiteralCell(literal.int64Value());
        case lyric_object::ValueType::Float64:
            return LiteralCell(literal.float64Value());
        case lyric_object::ValueType::Char:
            return LiteralCell(literal.charValue());
        case lyric_object::ValueType::String:
            return LiteralCell(literal.stringValue());
        default:
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "unknown literal type");
            return {};
    }
}