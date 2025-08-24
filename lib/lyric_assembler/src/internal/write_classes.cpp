
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/internal/write_classes.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Status
lyric_assembler::internal::touch_class(
    const ClassSymbol *classSymbol,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (classSymbol != nullptr);

    auto classUrl = classSymbol->getSymbolUrl();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertSymbol(classUrl, classSymbol, alreadyInserted));
    if (alreadyInserted)
        return {};

    // if class is an imported symbol then we are done
    if (classSymbol->isImported())
        return {};

    TU_RETURN_IF_NOT_OK (writer.touchType(classSymbol->classType()));

    auto *templateHandle = classSymbol->classTemplate();
    if (templateHandle) {
        TU_RETURN_IF_NOT_OK (writer.touchTemplate(templateHandle));
    }

    TU_RETURN_IF_NOT_OK (writer.touchConstructor(classSymbol->getCtor()));

    for (auto it = classSymbol->membersBegin(); it != classSymbol->membersEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchMember(it->second));
    }

    for (auto it = classSymbol->methodsBegin(); it != classSymbol->methodsEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchMethod(it->second));
    }

    for (auto it = classSymbol->implsBegin(); it != classSymbol->implsEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchImpl(it->second));
    }

    for (auto it = classSymbol->sealedTypesBegin(); it != classSymbol->sealedTypesEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchType(*it));
    }

    return {};
}

static tempo_utils::Status
write_class(
    const lyric_assembler::ClassSymbol *classSymbol,
    const lyric_assembler::ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::ClassDescriptor>> &classes_vector)
{
    auto classPathString = classSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(classPathString);

    tu_uint32 classType;
    TU_ASSIGN_OR_RETURN (classType, writer.getTypeOffset(classSymbol->classType()->getTypeDef()));

    tu_uint32 classTemplate = lyric_runtime::INVALID_ADDRESS_U32;
    if (classSymbol->classTemplate() != nullptr)
        TU_ASSIGN_OR_RETURN (classTemplate,
            writer.getTemplateOffset(classSymbol->classTemplate()->getTemplateUrl()));

    tu_uint32 superclassIndex = lyric_runtime::INVALID_ADDRESS_U32;
    auto *superclassSymbol = classSymbol->superClass();
    if (superclassSymbol != nullptr) {
        TU_ASSIGN_OR_RETURN (superclassIndex,
            writer.getSectionAddress(superclassSymbol->getSymbolUrl(), lyric_object::LinkageSection::Class));
    }

    lyo1::ClassFlags classFlags = lyo1::ClassFlags::NONE;
    if (classSymbol->isDeclOnly()) {
        classFlags |= lyo1::ClassFlags::DeclOnly;
    }
    if (classSymbol->isHidden()) {
        classFlags |= lyo1::ClassFlags::Hidden;
    }

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

    // serialize array of members
    std::vector<tu_uint32> members;
    for (auto iterator = classSymbol->membersBegin(); iterator != classSymbol->membersEnd(); iterator++) {
        const auto &memberRef = iterator->second;
        tu_uint32 fieldIndex;
        TU_ASSIGN_OR_RETURN (fieldIndex,
            writer.getSectionAddress(memberRef.symbolUrl, lyric_object::LinkageSection::Field));
        members.push_back(fieldIndex);
    }

    // serialize array of methods
    std::vector<tu_uint32> methods;
    for (auto iterator = classSymbol->methodsBegin(); iterator != classSymbol->methodsEnd(); iterator++) {
        const auto &boundMethod = iterator->second;
        tu_uint32 callIndex;
        TU_ASSIGN_OR_RETURN (callIndex,
            writer.getSectionAddress(boundMethod.methodCall, lyric_object::LinkageSection::Call));
        methods.push_back(callIndex);
    }

    // serialize array of impls
    std::vector<tu_uint32> impls;
    for (auto iterator = classSymbol->implsBegin(); iterator != classSymbol->implsEnd(); iterator++) {
        auto *implHandle = iterator->second;
        tu_uint32 implIndex;
        TU_ASSIGN_OR_RETURN (implIndex, writer.getImplOffset(implHandle->getRef()));
        impls.push_back(implIndex);
    }

    // get class ctor
    tu_uint32 ctorCall;
    TU_ASSIGN_OR_RETURN (ctorCall,
        writer.getSectionAddress(classSymbol->getCtor(), lyric_object::LinkageSection::Call));

    // serialize the sealed subtypes
    std::vector<tu_uint32> sealedSubtypes;
    for (auto iterator = classSymbol->sealedTypesBegin(); iterator != classSymbol->sealedTypesEnd(); iterator++) {
        tu_uint32 sealedSubtype;
        TU_ASSIGN_OR_RETURN (sealedSubtype, writer.getTypeOffset(*iterator));
        sealedSubtypes.push_back(sealedSubtype);
    }

    tu_uint32 allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    auto trapName = classSymbol->getAllocatorTrap();
    if (!trapName.empty()) {
        TU_ASSIGN_OR_RETURN (allocatorTrap, writer.getTrapNumber(trapName));
    }

    // add class descriptor
    classes_vector.push_back(lyo1::CreateClassDescriptor(buffer, fullyQualifiedName,
        superclassIndex, classTemplate, classType, classFlags,
        buffer.CreateVector(members), buffer.CreateVector(methods),
        buffer.CreateVector(impls), allocatorTrap, ctorCall,
        buffer.CreateVector(sealedSubtypes)));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_classes(
    const std::vector<const ClassSymbol *> &classes,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    ClassesOffset &classesOffset)
{
    std::vector<flatbuffers::Offset<lyo1::ClassDescriptor>> classes_vector;

    for (const auto *classSymbol : classes) {
        TU_RETURN_IF_NOT_OK (write_class(classSymbol, writer, buffer, classes_vector));
    }

    // create the classes vector
    classesOffset = buffer.CreateVector(classes_vector);

    return {};
}
