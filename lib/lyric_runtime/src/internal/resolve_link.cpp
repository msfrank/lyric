
#include <lyric_runtime/internal/resolve_link.h>

/**
 * Get the segment for the specified location. If no segment exists then attempt to load it. If load
 * succeeds then the segment is inserted into the segment cache. If the segment could not be loaded then
 * return nullptr to signal failure.
 *
 * @param objectLocation The location of the object
 * @param segmentManagerData Segment manager data
 * @return A pointer to the segment, otherwise nullptr if the segment could not be loaded.
 */
lyric_runtime::BytecodeSegment *
lyric_runtime::internal::get_or_load_segment(
    const lyric_common::ModuleLocation &location,
    const lyric_common::ModuleLocation &currentLocation,
    bool useSystemLoader,
    SegmentManagerData *segmentManagerData)
{
    // return null if assembly location is invalid
    if (!location.isValid())
        return nullptr;

    // if location is relative then resolve to an absolute location
    lyric_common::ModuleLocation objectLocation;
    if (location.isRelative()) {
        if (!currentLocation.isValid()) {
            TU_LOG_V << "failed to load " << location
                     << ": location is relative and current location is not specified";
            return nullptr;
        }
        objectLocation = currentLocation.resolve(location);
    } else {
        objectLocation = location;
    }

    auto loader = useSystemLoader?
        segmentManagerData->systemLoader : segmentManagerData->applicationLoader;

    // if segment is already loaded then return it
    auto entry = segmentManagerData->segmentcache.find(objectLocation);
    if (entry != segmentManagerData->segmentcache.cend())
        return segmentManagerData->segments[entry->second];

    auto loadModuleResult = loader->loadModule(objectLocation);
    if (loadModuleResult.isStatus()) {
        TU_LOG_V << "failed to load " << objectLocation << ": " << loadModuleResult.getStatus();
        return nullptr;                                 // failed to load assembly from location
    }
    auto objectOption = loadModuleResult.getResult();
    if (objectOption.isEmpty()) {
        TU_LOG_V << "failed to load " << objectLocation << ": object not found";
        return nullptr;                                 // failed to load assembly from location
    }

    auto object = objectOption.getValue();
    if (!object.isValid()) {
        TU_LOG_V << "failed to load " << objectLocation << ": object is invalid";
        return nullptr;                                 // failed to load assembly from location
    }

    // if module has a plugin then load it
    lyric_common::ModuleLocation pluginLocation;
    std::shared_ptr<const AbstractPlugin> plugin;
    if (object.hasPlugin()) {
        auto walker = object.getPlugin();

        pluginLocation = walker.getPluginLocation();
        if (pluginLocation.isValid()) {
            pluginLocation = objectLocation.resolve(pluginLocation);
        } else {
            pluginLocation = objectLocation;
        }

        auto loadPluginResult = loader->loadPlugin(
            pluginLocation, lyric_object::PluginSpecifier::systemDefault());
        if (loadPluginResult.isStatus()) {
            TU_LOG_V << "failed to load " << pluginLocation << ": " << loadPluginResult.getStatus();
            return nullptr;
        }
        auto pluginOption = loadPluginResult.getResult();
        if (pluginOption.isEmpty()) {
            TU_LOG_V << "failed to load " << pluginLocation << ": missing plugin";
            return nullptr;
        }
        plugin = pluginOption.getValue();
    }

    // allocate the segment
    auto segmentIndex = segmentManagerData->segments.size();
    auto *segment = new BytecodeSegment(segmentIndex, useSystemLoader, objectLocation, object, pluginLocation, plugin);

    // if there is a plugin then load it
    if (plugin != nullptr) {
        if (!plugin->load(segment)) {
            delete segment;
            return nullptr;
        }
    }

    // add the segment to the segment cache and return it
    segmentManagerData->segments.push_back(segment);
    segmentManagerData->segmentcache[objectLocation] = segmentIndex;
    return segment;
}

/**
 * Resolve the link at the specified index of the object in the current segment specified by `sp` and
 * return a pointer to the link entry. If the target segment does not exist in the cache then it will be
 * loaded as a side effect. If the index is invalid, or the target segment could not be loaded, or the
 * symbol could not be found within the target segment, then returns nullptr.
 *
 * @param sp The current segment
 * @param index The offset of the link in the current segment
 * @param segmentManagerData Segment manager data
 * @param status If the link could not be resolved then status is set
 * @return The resolved link entry, or nullptr if the link could not be resolved.
 */
const lyric_runtime::LinkEntry *
lyric_runtime::internal::resolve_link(
    const BytecodeSegment *sp,
    tu_uint32 index,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    // if there is no linkage then return null to signal error
    auto *linkageEntry = sp->getLink(index);
    if (linkageEntry == nullptr) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing linkage");
        return nullptr;
    }

    // if linkage section is not invalid, then dynamic linking has completed, so return the linkage entry
    if (linkageEntry->linkage != lyric_object::LinkageSection::Invalid)
        return linkageEntry;

    // get the link descriptor in the segment assembly
    auto currentObject = sp->getObject();
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
    auto location = referenceUrl.getModuleLocation();
    auto *segment = get_or_load_segment(location, sp->getObjectLocation(), sp->isSystem(), segmentManagerData);
    if (segment == nullptr) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kMissingObject, location.toString());
        return nullptr;
    }

    // get the symbol from the target assembly
    const auto targetObject = segment->getObject();
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
    completedLinkage.object = segment->getSegmentIndex();

    TU_LOG_V << "resolved " << symbolPath
             << " to descriptor " << completedLinkage.value
             << " in object " << completedLinkage.object;

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
        auto index = lyric_object::GET_LINK_OFFSET(address);
        const auto *linkage = resolve_link(sp, index, segmentManagerData, status);
        if (linkage == nullptr)
            return {};          // failed to resolve dynamic link
        if (linkage->linkage != section) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid linkage for descriptor");
            return {};          // wrong descriptor type
        }
        segmentIndex = linkage->object;
        valueIndex = linkage->value;
    }

    auto *segment = segmentManagerData->segments[segmentIndex];

    if (section == lyric_object::LinkageSection::Type) {
        auto *typeEntry = segment->lookupType(valueIndex);
        if (typeEntry == nullptr) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "missing type");
            return {};
        }
        return DataCell::forType(typeEntry);
    }

    auto *descriptorEntry = segment->lookupDescriptor(section, valueIndex);
    if (descriptorEntry == nullptr) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing descriptor");
        return {};
    }

    return DataCell::forDescriptor(descriptorEntry);
}

lyric_runtime::LiteralCell
lyric_runtime::internal::resolve_literal(
    const BytecodeSegment *sp,
    tu_uint32 address,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    TU_ASSERT (sp != nullptr);

    lyric_object::LyricObject literalObject;
    tu_uint32 literalIndex;

    if (lyric_object::IS_NEAR(address)) {
        literalObject = sp->getObject();
        literalIndex = lyric_object::GET_DESCRIPTOR_OFFSET(address);
    } else {
        auto index = lyric_object::GET_LINK_OFFSET(address);
        const auto *linkage = resolve_link(sp, index, segmentManagerData, status);
        if (linkage == nullptr)
            return {};          // failed to resolve dynamic link
        if (linkage->linkage != lyric_object::LinkageSection::Literal) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid linkage for literal");
            return {};          // wrong descriptor type
        }
        auto *segment = segmentManagerData->segments[linkage->object];
        literalObject = segment->getObject();
        literalIndex = linkage->value;
    }

    auto literal = literalObject.getLiteral(literalIndex);
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