
#include <tempo_utils/file_writer.h>

#include "lyric_assembler/call_symbol.h"
#include "lyric_assembler/object_root.h"
#include "lyric_assembler/object_state.h"
#include "lyric_bootstrap/bootstrap_helpers.h"
#include "lyric_bootstrap/bootstrap_loader.h"
#include "lyric_runtime/static_loader.h"

int main(int argc, char *argv[])
{
    // we expect a single argument which is the destination path where to write the object
    if (argc != 2)
        return -1;
    std::filesystem::path destinationPath(argv[1]);

    auto location = lyric_common::ModuleLocation::fromString("/testmod");
    auto origin = lyric_common::ModuleLocation::fromString("testmod://");

    auto localModuleCache = lyric_importer::ModuleCache::create(
        std::make_shared<lyric_runtime::StaticLoader>());
    auto systemModuleCache = lyric_importer::ModuleCache::create(
        std::make_shared<lyric_bootstrap::BootstrapLoader>());
    auto shortcutResolver = std::make_shared<lyric_importer::ShortcutResolver>();

    lyric_assembler::ObjectStateOptions options;
    lyric_assembler::ObjectState state(location, origin, localModuleCache, systemModuleCache, shortcutResolver, options);

    lyric_assembler::ObjectRoot *objectRoot;
    TU_ASSIGN_OR_RAISE (objectRoot, state.defineRoot());

    auto *entryCall = objectRoot->entryCall();
    auto *proc = entryCall->callProc();;
    auto *fragment = proc->procFragment();
    TU_RAISE_IF_NOT_OK (fragment->returnToCaller());
    TU_RAISE_IF_STATUS (entryCall->finalizeCall());

    lyric_object::LyricObject object;
    TU_ASSIGN_OR_RAISE (object, state.toObject());

    tempo_utils::FileWriter writer(destinationPath, object.bytesView(),
        tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
    if (!writer.isValid()) {
        TU_LOG_INFO << "failed to write output to " << destinationPath << "; " << writer.getStatus();
        return -1;
    }

    TU_LOG_INFO << "wrote output to " << destinationPath;
    return 0;
}
