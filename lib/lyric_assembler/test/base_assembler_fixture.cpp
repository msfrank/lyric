
#include <lyric_runtime/static_loader.h>

#include "base_assembler_fixture.h"

#include "lyric_assembler/call_symbol.h"
#include "lyric_assembler/object_root.h"
#include "lyric_assembler/object_writer.h"

void
BaseAssemblerFixture::SetUp()
{
    auto location = lyric_common::ModuleLocation::fromString("/test");
    auto staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
    auto bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    auto localModuleCache = lyric_importer::ModuleCache::create(staticLoader);
    auto systemModuleCache = lyric_importer::ModuleCache::create(bootstrapLoader);
    auto shortcutResolver = std::make_shared<lyric_importer::ShortcutResolver>();
    auto recorder = tempo_tracing::TraceRecorder::create();
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("tester://", tempo_utils::UUID::randomUUID().toString()));

    objectState = std::make_unique<lyric_assembler::ObjectState>(
        location, origin, localModuleCache, systemModuleCache, shortcutResolver);

    TU_ASSIGN_OR_RAISE (objectRoot, objectState->defineRoot());
}

tempo_utils::Status
BaseAssemblerFixture::writeObject(lyric_object::LyricObject &object)
{
    lyric_assembler::ObjectWriter objectWriter(objectState.get());
    TU_RAISE_IF_NOT_OK (objectWriter.initialize());
    TU_ASSIGN_OR_RETURN (object, objectWriter.toObject());
    return {};
}

tempo_utils::Status
BaseAssemblerFixture::writeObjectWithEmptyEntry(lyric_object::LyricObject &object)
{
    auto *entryCall = objectRoot->entryCall();
    auto *proc = entryCall->callProc();
    auto *root = proc->procFragment();
    TU_RAISE_IF_NOT_OK (root->returnToCaller());

    lyric_assembler::ObjectWriter objectWriter(objectState.get());
    TU_RAISE_IF_NOT_OK (objectWriter.initialize());
    TU_ASSIGN_OR_RETURN (object, objectWriter.toObject());
    return {};
}
