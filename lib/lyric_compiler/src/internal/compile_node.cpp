
#include <lyric_assembler/code_builder.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_assignment.h>
#include <lyric_compiler/internal/compile_block.h>
#include <lyric_compiler/internal/compile_conditional.h>
#include <lyric_compiler/internal/compile_def.h>
#include <lyric_compiler/internal/compile_defclass.h>
#include <lyric_compiler/internal/compile_defconcept.h>
#include <lyric_compiler/internal/compile_defenum.h>
#include <lyric_compiler/internal/compile_definstance.h>
#include <lyric_compiler/internal/compile_defstruct.h>
#include <lyric_compiler/internal/compile_deref.h>
#include <lyric_compiler/internal/compile_import.h>
#include <lyric_compiler/internal/compile_lambda.h>
#include <lyric_compiler/internal/compile_constant.h>
#include <lyric_compiler/internal/compile_loop.h>
#include <lyric_compiler/internal/compile_namespace.h>
#include <lyric_compiler/internal/compile_new.h>
#include <lyric_compiler/internal/compile_match.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_operator.h>
#include <lyric_compiler/internal/compile_return.h>
#include <lyric_compiler/internal/compile_using.h>
#include <lyric_compiler/internal/compile_val.h>
#include <lyric_compiler/internal/compile_var.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

/**
 *
 * @param builder
 * @param node
 * @return
 */
tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_expression(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId id{};
    moduleEntry.parseIdOrThrow(walker, lyric_schema::kLyricAstVocabulary, id);

    switch (id) {
        case lyric_schema::LyricAstId::Nil:
        case lyric_schema::LyricAstId::Undef:
        case lyric_schema::LyricAstId::True:
        case lyric_schema::LyricAstId::False:
        case lyric_schema::LyricAstId::Integer:
        case lyric_schema::LyricAstId::Float:
        case lyric_schema::LyricAstId::Char:
        case lyric_schema::LyricAstId::String:
        case lyric_schema::LyricAstId::Url:
        case lyric_schema::LyricAstId::SymbolRef:
            return compile_constant(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Seq:
        case lyric_schema::LyricAstId::Map:
        case lyric_schema::LyricAstId::Row:
            break;  // NOTE: implemented as data expression
        case lyric_schema::LyricAstId::IsEq:
        case lyric_schema::LyricAstId::IsLt:
        case lyric_schema::LyricAstId::IsLe:
        case lyric_schema::LyricAstId::IsGt:
        case lyric_schema::LyricAstId::IsGe:
        case lyric_schema::LyricAstId::IsA:
        case lyric_schema::LyricAstId::Add:
        case lyric_schema::LyricAstId::Sub:
        case lyric_schema::LyricAstId::Mul:
        case lyric_schema::LyricAstId::Div:
        case lyric_schema::LyricAstId::Neg:
        case lyric_schema::LyricAstId::And:
        case lyric_schema::LyricAstId::Or:
        case lyric_schema::LyricAstId::Not:
            return compile_operator(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Deref:
            return compile_deref(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Name:
            return compile_deref_name(block, block, walker, moduleEntry);
        case lyric_schema::LyricAstId::This:
            return compile_deref_this(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Call:
            return compile_deref_call(block, block, walker, moduleEntry);
        case lyric_schema::LyricAstId::New:
            return compile_new(block, walker, moduleEntry);
        //case lyric_schema::LyricAstId::Data:
        //    return compile_data(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Cond:
            return compile_cond(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Match:
            return compile_match(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Lambda:
            return compile_lambda(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Block:
            return compile_block(block, walker, moduleEntry);

        default:
            break;
    }

    block->throwSyntaxError(walker, "invalid expression");
}

tempo_utils::Status
lyric_compiler::internal::compile_statement(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId id{};
    moduleEntry.parseIdOrThrow(walker, lyric_schema::kLyricAstVocabulary, id);

    switch (id) {

        case lyric_schema::LyricAstId::Val:
            return compile_val(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Var:
            return compile_var(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Def:
            return compile_def(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::DefClass:
            return compile_defclass(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::DefConcept:
            return compile_defconcept(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::DefEnum:
            return compile_defenum(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::DefInstance:
            return compile_definstance(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::DefStruct:
            return compile_defstruct(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Namespace:
            return compile_namespace(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Set:
        case lyric_schema::LyricAstId::InplaceAdd:
        case lyric_schema::LyricAstId::InplaceSub:
        case lyric_schema::LyricAstId::InplaceMul:
        case lyric_schema::LyricAstId::InplaceDiv:
            return compile_set(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::If:
            return compile_if(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::While:
            return compile_while(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::For:
            return compile_for(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Return:
            return compile_return(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::ImportModule:
            return compile_import_module(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::ImportSymbols:
            return compile_import_symbols(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::ImportAll:
            return compile_import_all(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Using:
            return compile_using(block, walker, moduleEntry);

        default:
            break;
    }

    block->throwSyntaxError(walker, "invalid statement");
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_node(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId id{};
    moduleEntry.parseIdOrThrow(walker, lyric_schema::kLyricAstVocabulary, id);

    switch (id) {

        // expression forms
        case lyric_schema::LyricAstId::Nil:
        case lyric_schema::LyricAstId::Undef:
        case lyric_schema::LyricAstId::True:
        case lyric_schema::LyricAstId::False:
        case lyric_schema::LyricAstId::Integer:
        case lyric_schema::LyricAstId::Float:
        case lyric_schema::LyricAstId::Char:
        case lyric_schema::LyricAstId::String:
        case lyric_schema::LyricAstId::Url:
        case lyric_schema::LyricAstId::SymbolRef:
        case lyric_schema::LyricAstId::Seq:
        case lyric_schema::LyricAstId::Map:
        case lyric_schema::LyricAstId::Row:
        case lyric_schema::LyricAstId::Name:
        case lyric_schema::LyricAstId::This:
        case lyric_schema::LyricAstId::Call:
        case lyric_schema::LyricAstId::IsEq:
        case lyric_schema::LyricAstId::IsLt:
        case lyric_schema::LyricAstId::IsLe:
        case lyric_schema::LyricAstId::IsGt:
        case lyric_schema::LyricAstId::IsGe:
        case lyric_schema::LyricAstId::IsA:
        case lyric_schema::LyricAstId::Add:
        case lyric_schema::LyricAstId::Sub:
        case lyric_schema::LyricAstId::Mul:
        case lyric_schema::LyricAstId::Div:
        case lyric_schema::LyricAstId::Neg:
        case lyric_schema::LyricAstId::And:
        case lyric_schema::LyricAstId::Or:
        case lyric_schema::LyricAstId::Not:
        case lyric_schema::LyricAstId::Deref:
        case lyric_schema::LyricAstId::New:
        case lyric_schema::LyricAstId::Data:
        case lyric_schema::LyricAstId::Build:
        case lyric_schema::LyricAstId::Block:
        case lyric_schema::LyricAstId::Cond:
        case lyric_schema::LyricAstId::Match:
        case lyric_schema::LyricAstId::Lambda:
            return compile_expression(block, walker, moduleEntry);

        // statement forms
        case lyric_schema::LyricAstId::Val:
        case lyric_schema::LyricAstId::Var:
        case lyric_schema::LyricAstId::Def:
        case lyric_schema::LyricAstId::DefClass:
        case lyric_schema::LyricAstId::DefConcept:
        case lyric_schema::LyricAstId::DefEnum:
        case lyric_schema::LyricAstId::DefInstance:
        case lyric_schema::LyricAstId::DefStruct:
        case lyric_schema::LyricAstId::Namespace:
        case lyric_schema::LyricAstId::Generic:
        case lyric_schema::LyricAstId::Init:
        case lyric_schema::LyricAstId::Set:
        case lyric_schema::LyricAstId::InplaceAdd:
        case lyric_schema::LyricAstId::InplaceSub:
        case lyric_schema::LyricAstId::InplaceMul:
        case lyric_schema::LyricAstId::InplaceDiv:
        case lyric_schema::LyricAstId::If:
        case lyric_schema::LyricAstId::While:
        case lyric_schema::LyricAstId::For:
        case lyric_schema::LyricAstId::Return:
        case lyric_schema::LyricAstId::ImportModule:
        case lyric_schema::LyricAstId::ImportSymbols:
        case lyric_schema::LyricAstId::ImportAll:
        case lyric_schema::LyricAstId::Using:
        {
            auto status = compile_statement(block, walker, moduleEntry);
            if (status.notOk())
                return status;
            return lyric_common::TypeDef::noReturn();
        }

        default:
            break;
    }

    block->throwSyntaxError(walker, "invalid node");
}
