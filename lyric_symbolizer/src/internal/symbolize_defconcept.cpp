
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_symbolizer/internal/symbolize_defconcept.h>

static tempo_utils::Status
symbolize_defconcept_action(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT(walker.isValid());
    entryPoint.checkClassOrThrow(walker, lyric_schema::kLyricAstDefClass);

    std::string identifier;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);
    auto result = block->declareSymbol(identifier, lyric_object::LinkageSection::Action);
    if (result.isStatus())
        return result.getStatus();

    return lyric_symbolizer::SymbolizerStatus::ok();
}

tempo_utils::Status
lyric_symbolizer::internal::symbolize_defconcept(
    SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT(walker.isValid());
    entryPoint.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDefConceptClass, 1);

    std::string identifier;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);
    auto result = block->declareSymbol(identifier, lyric_object::LinkageSection::Concept);
    if (result.isStatus())
        return result.getStatus();
    SymbolizeHandle conceptBlock(result.getResult(), block);

    std::vector<lyric_parser::NodeWalker> actions;
    std::vector<lyric_parser::NodeWalker> impls;

    // make initial pass over concept body
    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        lyric_schema::LyricAstId childId{};
        entryPoint.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);

        switch (childId) {
            case lyric_schema::LyricAstId::Def:
                actions.emplace_back(child);
                break;
            case lyric_schema::LyricAstId::Impl:
                impls.emplace_back(child);
                break;
            default:
                block->throwSyntaxError(child, "expected concept body");
        }
    }

    for (const auto &action : actions) {
        auto status = symbolize_defconcept_action(&conceptBlock, action, entryPoint);
        if (!status.isOk())
            return status;
    }

    return lyric_symbolizer::SymbolizerStatus::ok();
}
