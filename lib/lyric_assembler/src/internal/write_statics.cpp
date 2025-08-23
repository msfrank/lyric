
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/internal/write_statics.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Status
lyric_assembler::internal::touch_static(
    const StaticSymbol *staticSymbol,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (staticSymbol != nullptr);

    auto staticUrl = staticSymbol->getSymbolUrl();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertSymbol(staticUrl, staticSymbol, alreadyInserted));
    if (alreadyInserted)
        return {};

    // if static is an imported symbol then we are done
    if (staticSymbol->isImported())
        return {};

    TU_RETURN_IF_NOT_OK (writer.touchType(staticSymbol->getTypeDef()));

    auto initializerUrl = staticSymbol->getInitializer();
    if (initializerUrl.isValid()) {
        TU_RETURN_IF_NOT_OK (writer.touchInitializer(initializerUrl));
    }

    return {};
}

static tempo_utils::Status
write_static(
    const lyric_assembler::StaticSymbol *staticSymbol,
    const lyric_assembler::ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::StaticDescriptor>> &statics_vector)
{
    auto staticPathString = staticSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fb_fullyQualifiedName = buffer.CreateSharedString(staticPathString);

    tu_uint32 staticType;
    TU_ASSIGN_OR_RETURN (staticType, writer.getTypeOffset(staticSymbol->getTypeDef()));

    lyo1::StaticFlags staticFlags = lyo1::StaticFlags::NONE;

    if (staticSymbol->isVariable()) {
        staticFlags |= lyo1::StaticFlags::Var;
    }
    if (staticSymbol->isHidden()) {
        staticFlags |= lyo1::StaticFlags::Hidden;
    }

    tu_uint32 initCall = lyric_object::INVALID_ADDRESS_U32;

    if (!staticSymbol->isDeclOnly()) {

        // get static initializer
        auto initializerUrl = staticSymbol->getInitializer();
        TU_ASSIGN_OR_RETURN (initCall,
            writer.getSectionAddress(initializerUrl, lyric_object::LinkageSection::Call));

    } else {
        staticFlags |= lyo1::StaticFlags::DeclOnly;
    }

    // add static descriptor
    statics_vector.push_back(lyo1::CreateStaticDescriptor(buffer,
        fb_fullyQualifiedName, staticType, staticFlags, initCall));

    return {};
}


tempo_utils::Status
lyric_assembler::internal::write_statics(
    const std::vector<const StaticSymbol *> &statics,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    StaticsOffset &staticsOffset)
{
    std::vector<flatbuffers::Offset<lyo1::StaticDescriptor>> statics_vector;

    for (const auto *staticSymbol : statics) {
        TU_RETURN_IF_NOT_OK (write_static(staticSymbol, writer, buffer, statics_vector));
    }

    // create the statics vector
    staticsOffset = buffer.CreateVector(statics_vector);

    return {};
}
