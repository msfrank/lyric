
#include <lyric_assembler/internal/write_statics.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

static tempo_utils::Status
write_static(
    lyric_assembler::StaticSymbol *staticSymbol,
    lyric_assembler::TypeCache *typeCache,
    lyric_assembler::SymbolCache *symbolCache,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::StaticDescriptor>> &statics_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    auto index = static_cast<tu_uint32>(statics_vector.size());

    auto staticPathString = staticSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fb_fullyQualifiedName = buffer.CreateSharedString(staticPathString);
    lyric_assembler::TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, typeCache->getOrMakeType(staticSymbol->getAssignableType()));
    tu_uint32 staticType = typeHandle->getAddress().getAddress();

    lyo1::StaticFlags staticFlags = lyo1::StaticFlags::NONE;

    if (!staticSymbol->getAddress().isValid())
        staticFlags |= lyo1::StaticFlags::DeclOnly;
    if (staticSymbol->isVariable())
        staticFlags |= lyo1::StaticFlags::Var;

    // get static initializer
    auto initializerUrl = staticSymbol->getInitializer();
    lyric_assembler::AbstractSymbol *initializer;
    TU_ASSIGN_OR_RETURN (initializer, symbolCache->getOrImportSymbol(initializerUrl));
    if (initializer->getSymbolType() != lyric_assembler::SymbolType::CALL)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing static init");
    auto initCall = cast_symbol_to_call(initializer)->getAddress().getAddress();

    // add static descriptor
    statics_vector.push_back(lyo1::CreateStaticDescriptor(buffer,
        fb_fullyQualifiedName, staticType, staticFlags, initCall));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fb_fullyQualifiedName,
        lyo1::DescriptorSection::Static, index));

    return {};
}


tempo_utils::Status
lyric_assembler::internal::write_statics(
    const AssemblyState *assemblyState,
    flatbuffers::FlatBufferBuilder &buffer,
    StaticsOffset &staticsOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    TU_ASSERT (assemblyState != nullptr);

    SymbolCache *symbolCache = assemblyState->symbolCache();
    TypeCache *typeCache = assemblyState->typeCache();
    std::vector<flatbuffers::Offset<lyo1::StaticDescriptor>> statics_vector;

    for (auto iterator = assemblyState->staticsBegin(); iterator != assemblyState->staticsEnd(); iterator++) {
        auto &staticSymbol = *iterator;
        TU_RETURN_IF_NOT_OK (
            write_static(staticSymbol, typeCache, symbolCache, buffer, statics_vector, symbols_vector));
    }

    // create the statics vector
    staticsOffset = buffer.CreateVector(statics_vector);

    return {};
}
