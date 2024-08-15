
#include <lyric_assembler/undeclared_symbol.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_symbolizer/internal/symbolize_defclass.h>
#include <lyric_symbolizer/internal/symbolize_defconcept.h>
#include <lyric_symbolizer/internal/symbolize_defenum.h>
#include <lyric_symbolizer/internal/symbolize_definstance.h>
#include <lyric_symbolizer/internal/symbolize_defstruct.h>
#include <lyric_symbolizer/internal/symbolize_handle.h>
#include <lyric_symbolizer/internal/symbolize_module.h>

tempo_utils::Status
lyric_symbolizer::internal::symbolize_block(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    entryPoint.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstBlockClass, 1);

    for (int i = 0; i < walker.numChildren(); i++) {
        auto status = symbolize_node(block, walker.getChild(i), entryPoint);
        if (!status.isOk())
            return status;
    }

    return lyric_symbolizer::SymbolizerStatus::ok();
}

static tempo_utils::Status
symbolize_namespace(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    entryPoint.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstNamespaceClass, 1);

    std::string identifier;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    auto result = block->declareSymbol(identifier, lyric_object::LinkageSection::Namespace);
    if (result.isStatus())
        return result.getStatus();
    lyric_symbolizer::internal::SymbolizeHandle nsBlock(result.getResult(), block);
    return symbolize_block(&nsBlock, walker.getChild(0), entryPoint);
}

static tempo_utils::Status
symbolize_val(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    entryPoint.checkClassOrThrow(walker, lyric_schema::kLyricAstValClass);

    // val is not a static
    if (block->blockParent() != nullptr)
        return lyric_symbolizer::SymbolizerStatus::ok();

    std::string identifier;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    auto result = block->declareSymbol(identifier, lyric_object::LinkageSection::Static);
    if (result.isStatus())
        return result.getStatus();
    return lyric_symbolizer::SymbolizerStatus::ok();
}

static tempo_utils::Status
symbolize_var(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    entryPoint.checkClassOrThrow(walker, lyric_schema::kLyricAstVarClass);

    // val is not a static
    if (block->blockParent() != nullptr)
        return lyric_symbolizer::SymbolizerStatus::ok();

    std::string identifier;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    auto result = block->declareSymbol(identifier, lyric_object::LinkageSection::Static);
    if (result.isStatus())
        return result.getStatus();
    return lyric_symbolizer::SymbolizerStatus::ok();
}

static tempo_utils::Status
symbolize_def(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    entryPoint.checkClassOrThrow(walker, lyric_schema::kLyricAstDefClass);

    std::string identifier;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    auto result = block->declareSymbol(identifier, lyric_object::LinkageSection::Call);
    if (result.isStatus())
        return result.getStatus();
    lyric_symbolizer::internal::SymbolizeHandle callBlock(result.getResult(), block);
    return symbolize_block(&callBlock, walker.getChild(1), entryPoint);
}

static tempo_utils::Status
symbolize_while(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    entryPoint.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstWhileClass, 2);

    auto status = symbolize_node(block, walker.getChild(0), entryPoint);
    if (!status.isOk())
        return status;
    return symbolize_block(block, walker.getChild(1), entryPoint);
}

static tempo_utils::Status
symbolize_for(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    entryPoint.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstForClass, 2);

    auto status = symbolize_node(block, walker.getChild(0), entryPoint);
    if (!status.isOk())
        return status;
    return symbolize_block(block, walker.getChild(1), entryPoint);
}

static tempo_utils::Status
symbolize_cond(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    entryPoint.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstCondClass, 1);

    tempo_utils::Status status;
    for (int i = 0; i < walker.numChildren(); i++) {
        auto condCase = walker.getChild(i);
        entryPoint.checkClassAndChildCountOrThrow(condCase, lyric_schema::kLyricAstCaseClass, 2);
        status = symbolize_node(block, condCase.getChild(0), entryPoint);
        if (!status.isOk())
            return status;
        status = symbolize_node(block, condCase.getChild(1), entryPoint);
        if (!status.isOk())
            return status;
    }

    if (walker.hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        lyric_parser::NodeWalker defaultNode;
        entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstDefaultOffset, defaultNode);
        status = symbolize_node(block, defaultNode, entryPoint);
        if (!status.isOk())
            return status;
    }

    return lyric_symbolizer::SymbolizerStatus::ok();
}

static tempo_utils::Status
symbolize_if(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    entryPoint.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstIfClass, 1);

    tempo_utils::Status status;
    for (int i = 0; i < walker.numChildren(); i++) {
        auto condCase = walker.getChild(i);
        entryPoint.checkClassAndChildCountOrThrow(condCase, lyric_schema::kLyricAstCaseClass, 2);
        status = symbolize_node(block, condCase.getChild(0), entryPoint);
        if (!status.isOk())
            return status;
        status = symbolize_node(block, condCase.getChild(1), entryPoint);
        if (!status.isOk())
            return status;
    }

    if (walker.hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        lyric_parser::NodeWalker defaultNode;
        entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstDefaultOffset, defaultNode);
        status = symbolize_node(block, defaultNode, entryPoint);
        if (!status.isOk())
            return status;
    }

    return lyric_symbolizer::SymbolizerStatus::ok();
}

static tempo_utils::Status
symbolize_match(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    entryPoint.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstMatchClass, 1);

    tempo_utils::Status status = symbolize_node(block, walker.getChild(0), entryPoint);
    if (!status.isOk())
        return status;

    for (int i = 1; i < walker.numChildren(); i++) {
        auto matchCase = walker.getChild(i);
        entryPoint.checkClassAndChildCountOrThrow(matchCase, lyric_schema::kLyricAstCaseClass, 2);
        status = symbolize_node(block, matchCase.getChild(0), entryPoint);
        if (!status.isOk())
            return status;
        status = symbolize_node(block, matchCase.getChild(1), entryPoint);
        if (!status.isOk())
            return status;
    }

    if (walker.hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        lyric_parser::NodeWalker defaultNode;
        entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstDefaultOffset, defaultNode);
        status = symbolize_block(block, defaultNode, entryPoint);
        if (!status.isOk())
            return status;
    }

    return lyric_symbolizer::SymbolizerStatus::ok();
}

static tempo_utils::Status
symbolize_deref(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    entryPoint.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDerefClass, 1);

    for (int i = 0; i < walker.numChildren(); i++) {
        auto status = symbolize_node(block, walker.getChild(i), entryPoint);
        if (!status.isOk())
            return status;
    }

    return lyric_symbolizer::SymbolizerStatus::ok();
}

static tempo_utils::Status
symbolize_set(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());

    auto status = symbolize_node(block, walker.getChild(0), entryPoint);
    if (!status.isOk())
        return status;
    return symbolize_node(block, walker.getChild(1), entryPoint);
}

static tempo_utils::Status
symbolize_return(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    entryPoint.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstReturnClass, 1);
    return symbolize_node(block, walker.getChild(0), entryPoint);
}

static tempo_utils::Status
symbolize_keyword(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    return symbolize_node(block, walker.getChild(0), entryPoint);
}

static tempo_utils::Status
symbolize_callable_unary(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    return symbolize_node(block, walker.getChild(0), entryPoint);
}

static tempo_utils::Status
symbolize_callable_binary(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    auto status = symbolize_node(block, walker.getChild(0), entryPoint);
    if (!status.isOk())
        return status;
    return symbolize_node(block, walker.getChild(1), entryPoint);
}

static tempo_utils::Status
symbolize_callable_nary(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    for (int i = 0; i < walker.numChildren(); i++) {
        auto status = symbolize_node(block, walker.getChild(i), entryPoint);
        if (!status.isOk())
            return status;
    }
    return lyric_symbolizer::SymbolizerStatus::ok();
}

static tempo_utils::Status
symbolize_unwrap(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    return lyric_symbolizer::SymbolizerStatus::ok();
}

static tempo_utils::Status
symbolize_import(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_symbolizer::internal::EntryPoint &entryPoint)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());
    lyric_common::ModuleLocation importLocation;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstModuleLocation, importLocation);
    return block->declareImport(importLocation);
}

tempo_utils::Status
lyric_symbolizer::internal::symbolize_node(
    lyric_symbolizer::internal::SymbolizeHandle *block,
    const lyric_parser::NodeWalker &walker,
    EntryPoint &entryPoint)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId nodeId{};
    entryPoint.parseIdOrThrow(walker, lyric_schema::kLyricAstVocabulary, nodeId);

    switch (nodeId) {

        // ignored terminal forms
        case lyric_schema::LyricAstId::Nil:
        case lyric_schema::LyricAstId::Undef:
        case lyric_schema::LyricAstId::False:
        case lyric_schema::LyricAstId::True:
        case lyric_schema::LyricAstId::Integer:
        case lyric_schema::LyricAstId::Float:
        case lyric_schema::LyricAstId::Char:
        case lyric_schema::LyricAstId::String:
        case lyric_schema::LyricAstId::Url:
        case lyric_schema::LyricAstId::SymbolRef:
        case lyric_schema::LyricAstId::This:
        case lyric_schema::LyricAstId::Name:
        case lyric_schema::LyricAstId::Target:
        case lyric_schema::LyricAstId::Using:
            return lyric_symbolizer::SymbolizerStatus::ok();

        // handled forms
        case lyric_schema::LyricAstId::Block:
            return symbolize_block(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Val:
            return symbolize_val(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Var:
            return symbolize_var(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Def:
            return symbolize_def(block, walker, entryPoint);
        case lyric_schema::LyricAstId::DefClass:
            return symbolize_defclass(block, walker, entryPoint);
        case lyric_schema::LyricAstId::DefConcept:
            return symbolize_defconcept(block, walker, entryPoint);
        case lyric_schema::LyricAstId::DefEnum:
            return symbolize_defenum(block, walker, entryPoint);
        case lyric_schema::LyricAstId::DefInstance:
            return symbolize_definstance(block, walker, entryPoint);
        case lyric_schema::LyricAstId::DefStruct:
            return symbolize_defstruct(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Deref:
            return symbolize_deref(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Set:
        case lyric_schema::LyricAstId::InplaceAdd:
        case lyric_schema::LyricAstId::InplaceSub:
        case lyric_schema::LyricAstId::InplaceMul:
        case lyric_schema::LyricAstId::InplaceDiv:
            return symbolize_set(block, walker, entryPoint);
        case lyric_schema::LyricAstId::While:
            return symbolize_while(block, walker, entryPoint);
        case lyric_schema::LyricAstId::For:
            return symbolize_for(block, walker, entryPoint);
        case lyric_schema::LyricAstId::If:
            return symbolize_if(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Cond:
            return symbolize_cond(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Match:
            return symbolize_match(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Return:
            return symbolize_return(block, walker, entryPoint);
        case lyric_schema::LyricAstId::IsA:
        case lyric_schema::LyricAstId::Neg:
        case lyric_schema::LyricAstId::Not:
            return symbolize_callable_unary(block, walker, entryPoint);
        case lyric_schema::LyricAstId::IsEq:
        case lyric_schema::LyricAstId::IsLt:
        case lyric_schema::LyricAstId::IsLe:
        case lyric_schema::LyricAstId::IsGt:
        case lyric_schema::LyricAstId::IsGe:
        case lyric_schema::LyricAstId::Add:
        case lyric_schema::LyricAstId::Sub:
        case lyric_schema::LyricAstId::Mul:
        case lyric_schema::LyricAstId::Div:
        case lyric_schema::LyricAstId::And:
        case lyric_schema::LyricAstId::Or:
            return symbolize_callable_binary(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Call:
        case lyric_schema::LyricAstId::New:
        case lyric_schema::LyricAstId::Super:
            return symbolize_callable_nary(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Unpack:
            return symbolize_unwrap(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Keyword:
            return symbolize_keyword(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Namespace:
            return symbolize_namespace(block, walker, entryPoint);
        case lyric_schema::LyricAstId::ImportModule:
        case lyric_schema::LyricAstId::ImportSymbols:
        case lyric_schema::LyricAstId::ImportAll:
            return symbolize_import(block, walker, entryPoint);

        // unhandled forms
        case lyric_schema::LyricAstId::Lambda:
        case lyric_schema::LyricAstId::Generic:
        default:
            break;
    }

    block->throwSyntaxError(walker, "invalid node");
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_symbolizer::internal::symbolize_module(
    const lyric_parser::NodeWalker &walker,
    EntryPoint &entryPoint)
{
    TU_ASSERT (walker.isValid());
    auto *state = entryPoint.getState();

    // define entry for toplevel code
    lyric_symbolizer::internal::SymbolizeHandle entryBlock(state);

    // scan assembly starting at entry node
    auto status = symbolize_node(&entryBlock, walker, entryPoint);
    if (!status.isOk())
        return status;

    // if any statics were declared, then add the $entry symbol
    for (auto iterator = state->undeclaredBegin(); iterator != state->undeclaredEnd(); iterator++) {
        auto *undecl = *iterator;
        if (undecl->getLinkage() == lyric_object::LinkageSection::Static) {
            entryBlock.declareSymbol("$entry", lyric_object::LinkageSection::Call);
            break;
        }
    }

    // construct assembly from assembly state and return it
    auto toObjectResult = state->toObject();
    if (toObjectResult.isStatus())
        return toObjectResult.getStatus();
    return toObjectResult.getResult();
}
