
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/internal/write_bindings.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/symbol_cache.h>

tempo_utils::Status
lyric_assembler::internal::touch_binding(
    const BindingSymbol *bindingSymbol,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (bindingSymbol != nullptr);

    auto bindingUrl = bindingSymbol->getSymbolUrl();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertSymbol(bindingUrl, bindingSymbol, alreadyInserted));
    if (alreadyInserted)
        return {};

    // if binding is an imported symbol then we are done
    if (bindingSymbol->isImported())
        return {};

    TU_RETURN_IF_NOT_OK (writer.touchType(bindingSymbol->bindingType()));

    auto *templateHandle = bindingSymbol->bindingTemplate();
    if (templateHandle) {
        TU_RETURN_IF_NOT_OK (writer.touchTemplate(templateHandle));
    }

    TU_RETURN_IF_NOT_OK (writer.touchType(bindingSymbol->targetType()));

    return {};
}

static tempo_utils::Status
write_binding(
    const lyric_assembler::BindingSymbol *bindingSymbol,
    const lyric_assembler::ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::BindingDescriptor>> &bindings_vector)
{
    auto bindingPathString = bindingSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fb_fullyQualifiedName = buffer.CreateSharedString(bindingPathString);

    lyo1::BindingFlags bindingFlags = lyo1::BindingFlags::NONE;
    if (bindingSymbol->isHidden()) {
        bindingFlags |= lyo1::BindingFlags::Hidden;
    }

    tu_uint32 bindingType;
    TU_ASSIGN_OR_RETURN (bindingType, writer.getTypeOffset(bindingSymbol->bindingType()->getTypeDef()));

    tu_uint32 bindingTemplate = lyric_runtime::INVALID_ADDRESS_U32;
    if (bindingSymbol->bindingTemplate() != nullptr)
        TU_ASSIGN_OR_RETURN (bindingTemplate,
            writer.getTemplateOffset(bindingSymbol->bindingTemplate()->getTemplateUrl()));

    tu_uint32 targetType;
    TU_ASSIGN_OR_RETURN (targetType, writer.getTypeOffset(bindingSymbol->targetType()->getTypeDef()));

    // add binding descriptor
    bindings_vector.push_back(lyo1::CreateBindingDescriptor(buffer,
        fb_fullyQualifiedName, bindingTemplate, bindingType, bindingFlags, targetType));

    return {};
}


tempo_utils::Status
lyric_assembler::internal::write_bindings(
    const std::vector<const BindingSymbol *> &bindings,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    BindingsOffset &bindingsOffset)
{
    std::vector<flatbuffers::Offset<lyo1::BindingDescriptor>> bindings_vector;

    for (const auto *bindingSymbol : bindings) {
        TU_RETURN_IF_NOT_OK (write_binding(bindingSymbol, writer, buffer, bindings_vector));
    }

    // create the bindings vector
    bindingsOffset = buffer.CreateVector(bindings_vector);

    return {};
}
