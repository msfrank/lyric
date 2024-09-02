//
//#include <lyric_parser/ast_attrs.h>
//#include <lyric_schema/ast_schema.h>
//
//#include <lyric_symbolizer/internal/symbolize_definstance.h>
//#include <lyric_symbolizer/internal/symbolize_module.h>
//
//static tempo_utils::Status
//symbolize_definstance_def(
//    lyric_symbolizer::internal::SymbolizeHandle *block,
//    const lyric_parser::NodeWalker &walker,
//    lyric_symbolizer::internal::EntryPoint &entryPoint)
//{
//    TU_ASSERT(block != nullptr);
//    TU_ASSERT(walker.isValid());
//    entryPoint.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDefClass, 2);
//
//    std::string identifier;
//    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);
//    auto result = block->declareSymbol(identifier, lyric_object::LinkageSection::Call);
//    if (result.isStatus())
//        return result.getStatus();
//    lyric_symbolizer::internal::SymbolizeHandle callBlock(result.getResult(), block);
//    return symbolize_block(&callBlock, walker.getChild(1), entryPoint);
//}
//
//tempo_utils::Status
//lyric_symbolizer::internal::symbolize_definstance(
//    SymbolizeHandle *block,
//    const lyric_parser::NodeWalker &walker,
//    EntryPoint &entryPoint)
//{
//    TU_ASSERT(block != nullptr);
//    TU_ASSERT(walker.isValid());
//    entryPoint.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDefInstanceClass, 1);
//
//    std::string identifier;
//    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);
//    auto result = block->declareSymbol(identifier, lyric_object::LinkageSection::Instance);
//    if (result.isStatus())
//        return result.getStatus();
//    SymbolizeHandle instanceBlock(result.getResult(), block);
//
//    lyric_parser::NodeWalker init;
//    std::vector<lyric_parser::NodeWalker> vals;
//    std::vector<lyric_parser::NodeWalker> vars;
//    std::vector<lyric_parser::NodeWalker> defs;
//    std::vector<lyric_parser::NodeWalker> impls;
//
//    // make initial pass over instance body
//    for (int i = 0; i < walker.numChildren(); i++) {
//        auto child = walker.getChild(i);
//        lyric_schema::LyricAstId childId{};
//        entryPoint.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);
//
//        switch (childId) {
//            case lyric_schema::LyricAstId::Val:
//                vals.emplace_back(child);
//                break;
//            case lyric_schema::LyricAstId::Var:
//                vars.emplace_back(child);
//                break;
//            case lyric_schema::LyricAstId::Def:
//                defs.emplace_back(child);
//                break;
//            case lyric_schema::LyricAstId::Impl:
//                impls.emplace_back(child);
//                break;
//            default:
//                block->throwSyntaxError(child, "expected instance body");
//        }
//    }
//
//    tempo_utils::Status status;
//
//    for (const auto &def : defs) {
//        status = symbolize_definstance_def(&instanceBlock, def, entryPoint);
//        if (!status.isOk())
//            return status;
//    }
//
//    return lyric_symbolizer::SymbolizerStatus::ok();
//}
