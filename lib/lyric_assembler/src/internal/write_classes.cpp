
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/internal/write_classes.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

static tempo_utils::Status
write_class(
    lyric_assembler::ClassSymbol *classSymbol,
    lyric_assembler::TypeCache *typeCache,
    lyric_assembler::SymbolCache *symbolCache,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::ClassDescriptor>> &classes_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    auto index = static_cast<tu_uint32>(classes_vector.size());

    auto classPathString = classSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(classPathString);
    auto typeIndex = classSymbol->classType()->getAddress().getAddress();

    tu_uint32 classTemplate = lyric_runtime::INVALID_ADDRESS_U32;
    if (classSymbol->classTemplate() != nullptr)
        classTemplate = classSymbol->classTemplate()->getAddress().getAddress();

    auto superclassIndex = lyric_runtime::INVALID_ADDRESS_U32;
    auto *superclassSymbol = classSymbol->superClass();
    if (superclassSymbol != nullptr) {
        superclassIndex = superclassSymbol->getAddress().getAddress();
    }

    lyo1::ClassFlags classFlags = lyo1::ClassFlags::NONE;
    if (classSymbol->isDeclOnly())
        classFlags |= lyo1::ClassFlags::DeclOnly;
    if (classSymbol->isAbstract())
        classFlags |= lyo1::ClassFlags::Abstract;
    switch (classSymbol->getDeriveType()) {
        case lyric_object::DeriveType::Final:
            classFlags |= lyo1::ClassFlags::Final;
            break;
        case lyric_object::DeriveType::Sealed:
            classFlags |= lyo1::ClassFlags::Sealed;
            break;
        default:
            break;
    }
    switch (classSymbol->getAccessType()) {
        case lyric_object::AccessType::Public:
            classFlags |= lyo1::ClassFlags::GlobalVisibility;
            break;
        case lyric_object::AccessType::Protected:
            classFlags |= lyo1::ClassFlags::InheritVisibility;
            break;
        case lyric_object::AccessType::Private:
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                "invalid class access");
    }

    // serialize array of members
    std::vector<tu_uint32> members;
    for (auto iterator = classSymbol->membersBegin(); iterator != classSymbol->membersEnd(); iterator++) {
        const auto &var = iterator->second;
        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(var.symbolUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::FIELD)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid field symbol");
        auto *fieldSymbol = cast_symbol_to_field(symbol);

        members.push_back(fieldSymbol->getAddress().getAddress());
    }

    // serialize array of methods
    std::vector<tu_uint32> methods;
    for (auto iterator = classSymbol->methodsBegin(); iterator != classSymbol->methodsEnd(); iterator++) {
        const auto &boundMethod = iterator->second;
        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(boundMethod.methodCall));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid call symbol");
        const auto *callSymbol = cast_symbol_to_call(symbol);

        methods.push_back(callSymbol->getAddress().getAddress());
    }

    // serialize array of impls
    std::vector<tu_uint32> impls;
    for (auto iterator = classSymbol->implsBegin(); iterator != classSymbol->implsEnd(); iterator++) {
        auto *implHandle = iterator->second;
        impls.push_back(implHandle->getOffset().getOffset());
    }

    // get class ctor
    auto ctorUrl = classSymbol->getCtor();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ctorUrl));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing class ctor");
    auto ctorCall = cast_symbol_to_call(symbol)->getAddress().getAddress();

    // serialize the sealed subtypes
    std::vector<tu_uint32> sealedSubtypes;
    for (auto iterator = classSymbol->sealedTypesBegin(); iterator != classSymbol->sealedTypesEnd(); iterator++) {
        if (!typeCache->hasType(*iterator))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing sealed subtype");
        lyric_assembler::TypeHandle *typeHandle;
        TU_ASSIGN_OR_RETURN (typeHandle, typeCache->getOrMakeType(*iterator));
        sealedSubtypes.push_back(typeHandle->getAddress().getAddress());
    }

    // add class descriptor
    classes_vector.push_back(lyo1::CreateClassDescriptor(buffer, fullyQualifiedName,
        superclassIndex, classTemplate, typeIndex, classFlags,
        buffer.CreateVector(members), buffer.CreateVector(methods), buffer.CreateVector(impls),
        classSymbol->getAllocatorTrap(), ctorCall, buffer.CreateVector(sealedSubtypes)));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Class, index));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_classes(
    const ObjectState *objectState,
    flatbuffers::FlatBufferBuilder &buffer,
    ClassesOffset &classesOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    TU_ASSERT (objectState != nullptr);

    SymbolCache *symbolCache = objectState->symbolCache();
    TypeCache *typeCache = objectState->typeCache();
    std::vector<flatbuffers::Offset<lyo1::ClassDescriptor>> classes_vector;

    for (auto iterator = objectState->classesBegin(); iterator != objectState->classesEnd(); iterator++) {
        auto &classSymbol = *iterator;
        TU_RETURN_IF_NOT_OK (
            write_class(classSymbol, typeCache, symbolCache, buffer, classes_vector, symbols_vector));
    }

    // create the classes vector
    classesOffset = buffer.CreateVector(classes_vector);

    return {};
}
