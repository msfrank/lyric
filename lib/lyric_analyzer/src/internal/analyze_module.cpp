
#include <lyric_analyzer/internal/analyze_module.h>
#include <lyric_analyzer/internal/analyze_node.h>

tempo_utils::Result<lyric_object::LyricObject>
lyric_analyzer::internal::analyze_module(
    const lyric_parser::NodeWalker &walker,
    EntryPoint &entryPoint)
{
    auto *root = entryPoint.getRoot();
    auto *rootBlock = root->namespaceBlock();

    // scan assembly starting at entry node
    auto status = analyze_node(rootBlock, walker, entryPoint);
    if (!status.isOk())
        return status;

    auto *state = entryPoint.getState();

    // construct assembly from assembly state and return it
    auto toObjectResult = state->toObject();
    if (toObjectResult.isStatus())
        return toObjectResult.getStatus();
    return toObjectResult.getResult();
}
