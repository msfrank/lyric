
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

    auto targetUrl = bindingSymbol->getTargetUrl();
    auto *targetSymbol = objectState->symbolCache()->getSymbolOrNull(targetUrl);
    switch (targetSymbol->getSymbolType()) {
        case SymbolType::ACTION:
            TU_RETURN_IF_NOT_OK (writer.touchAction(cast_symbol_to_action(targetSymbol)));
            break;
        case SymbolType::CALL:
            TU_RETURN_IF_NOT_OK (writer.touchCall(cast_symbol_to_call(targetSymbol)));
            break;
        case SymbolType::CLASS:
            TU_RETURN_IF_NOT_OK (writer.touchClass(cast_symbol_to_class(targetSymbol)));
            break;
        case SymbolType::CONCEPT:
            TU_RETURN_IF_NOT_OK (writer.touchConcept(cast_symbol_to_concept(targetSymbol)));
            break;
        case SymbolType::ENUM:
            TU_RETURN_IF_NOT_OK (writer.touchEnum(cast_symbol_to_enum(targetSymbol)));
            break;
        case SymbolType::EXISTENTIAL:
            TU_RETURN_IF_NOT_OK (writer.touchExistential(cast_symbol_to_existential(targetSymbol)));
            break;
        case SymbolType::FIELD:
            TU_RETURN_IF_NOT_OK (writer.touchField(cast_symbol_to_field(targetSymbol)));
            break;
        case SymbolType::INSTANCE:
            TU_RETURN_IF_NOT_OK (writer.touchInstance(cast_symbol_to_instance(targetSymbol)));
            break;
        case SymbolType::NAMESPACE:
            TU_RETURN_IF_NOT_OK (writer.touchNamespace(cast_symbol_to_namespace(targetSymbol)));
            break;
        case SymbolType::STATIC:
            TU_RETURN_IF_NOT_OK (writer.touchStatic(cast_symbol_to_static(targetSymbol)));
            break;
        case SymbolType::STRUCT:
            TU_RETURN_IF_NOT_OK (writer.touchStruct(cast_symbol_to_struct(targetSymbol)));
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                "invalid target symbol for binding {}", bindingUrl.toString());
    }

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

    switch (bindingSymbol->getAccessType()) {
        case lyric_object::AccessType::Public:
            bindingFlags |= lyo1::BindingFlags::GlobalVisibility;
            break;
        case lyric_object::AccessType::Protected:
            bindingFlags |= lyo1::BindingFlags::InheritVisibility;
            break;
        case lyric_object::AccessType::Private:
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                "invalid binding access");
    }

    auto targetUrl = bindingSymbol->getTargetUrl();
    tu_uint32 targetAddress;
    TU_ASSIGN_OR_RETURN (targetAddress, writer.getSymbolAddress(targetUrl));

    // add binding descriptor
    bindings_vector.push_back(lyo1::CreateBindingDescriptor(buffer,
        fb_fullyQualifiedName, targetAddress, bindingFlags));

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
