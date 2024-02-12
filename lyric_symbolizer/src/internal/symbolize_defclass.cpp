
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_symbolizer/internal/symbolize_defclass.h>
#include <lyric_symbolizer/internal/symbolize_module.h>

static tempo_utils::Status
symbolize_defclass_init(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &initPack,
    const lyric_parser::NodeWalker &initSuper,
    const lyric_parser::NodeWalker &initBody,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);

    auto result = block->declareSymbol("$ctor", lyric_object::LinkageSection::Call);
    if (result.isStatus())
        return result.getStatus();
    lyric_symbolizer::internal::SymbolizeHandle callBlock(result.getResult(), block);

    if (initSuper.isValid()) {
        auto status = symbolize_node(&callBlock, initSuper, entryPoint);
        if (!status.isOk())
            return status;
    }
    if (initBody.isValid())
        return symbolize_block(&callBlock, initBody, entryPoint);

    return lyric_symbolizer::SymbolizerStatus::ok();
}

static tempo_utils::Status
symbolize_defclass_def(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT(walker.isValid());
    entryPoint.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDefClass, 2);

    std::string identifier;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);
    auto result = block->declareSymbol(identifier, lyric_object::LinkageSection::Call);
    if (result.isStatus())
        return result.getStatus();
    lyric_symbolizer::internal::SymbolizeHandle callBlock(result.getResult(), block);

    return symbolize_block(&callBlock, walker.getChild(1), entryPoint);
}

tempo_utils::Status
lyric_symbolizer::internal::symbolize_defclass(
    SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT(walker.isValid());
    entryPoint.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDefClassClass, 1);

    std::string identifier;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);
    auto result = block->declareSymbol(identifier, lyric_object::LinkageSection::Class);
    if (result.isStatus())
        return result.getStatus();
    SymbolizeHandle classBlock(result.getResult(), block);

    lyric_parser::NodeWalker init;
    std::vector<lyric_parser::NodeWalker> vals;
    std::vector<lyric_parser::NodeWalker> vars;
    std::vector<lyric_parser::NodeWalker> defs;

    // make initial pass over class body
    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        lyric_schema::LyricAstId childId{};
        entryPoint.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);

        switch (childId) {
            case lyric_schema::LyricAstId::Init:
                if (init.isValid())
                    block->throwSyntaxError(child, "invalid init");
                init = child;
                break;
            case lyric_schema::LyricAstId::Val:
                vals.emplace_back(child);
                break;
            case lyric_schema::LyricAstId::Var:
                vars.emplace_back(child);
                break;
            case lyric_schema::LyricAstId::Def:
                defs.emplace_back(child);
                break;
            default:
                block->throwSyntaxError(child, "expected class body");
        }
    }

    lyric_parser::NodeWalker initPack;
    lyric_parser::NodeWalker initSuper;
    lyric_parser::NodeWalker initBody;

    if (init.isValid()) {
        entryPoint.checkClassAndChildRangeOrThrow(init, lyric_schema::kLyricAstInitClass, 1, 3);

        initPack = init.getChild(0);
        entryPoint.checkClassOrThrow(initPack, lyric_schema::kLyricAstPackClass);

        if (init.numChildren() == 2) {
            auto child = init.getChild(1);
            lyric_schema::LyricAstId childId;
            entryPoint.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);
            switch (childId) {
                case lyric_schema::LyricAstId::Super:
                    initSuper = child;
                    break;
                case lyric_schema::LyricAstId::Block:
                    initBody = child;
                    break;
                default:
                    block->throwSyntaxError(init, "invalid init");
            }
        } else if (init.numChildren() == 3) {
            initSuper = init.getChild(1);
            entryPoint.checkClassOrThrow(initSuper, lyric_schema::kLyricAstSuperClass);
            initBody = init.getChild(2);
            entryPoint.checkClassOrThrow(initBody, lyric_schema::kLyricAstBlockClass);
        }
    }

    auto status = symbolize_defclass_init(&classBlock, initPack, initSuper, initBody, entryPoint);
    if (!status.isOk())
        return status;

    for (const auto &def : defs) {
        status = symbolize_defclass_def(&classBlock, def, entryPoint);
        if (!status.isOk())
            return status;
    }
    return lyric_symbolizer::SymbolizerStatus::ok();
}