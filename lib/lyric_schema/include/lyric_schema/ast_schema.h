#ifndef LYRIC_SCHEMA_AST_SCHEMA_H
#define LYRIC_SCHEMA_AST_SCHEMA_H

#include <tempo_utils/schema.h>

namespace lyric_schema {

    class LyricAstNs : public tempo_utils::SchemaNs {
    public:
        constexpr LyricAstNs() : tempo_utils::SchemaNs("dev.zuri.ns:ast-1") {};
    };
    constexpr LyricAstNs kLyricAstNs;

    enum class LyricAstId {

        //
        Nil,                       // nil literal
        Undef,                     // undef literal
        False,                     // false literal
        True,                      // true literal
        Integer,                   // integer literal
        Float,                     // float literal
        Char,                      // char literal
        String,                    // string literal
        Url,                       // url literal

        Pair,                      // pair container
        Seq,                       // seq container
        Map,                       // map container
        Row,                       // row container

        Add,                       // addition operator
        Sub,                       // subtraction operator
        Mul,                       // multiplication operator
        Div,                       // division operator
        Neg,                       // additive inverse operator
        IsEq,                      // equals operator
        IsLt,                      // less-than operator
        IsLe,                      // less-than-or-equals operator
        IsGt,                      // greater-than operator
        IsGe,                      // greater-than-or-equals operator
        IsA,                       // type-equals operator

        And,                       // logical conjunction operator
        Or,                        // logical disjunction operator
        Not,                       // logical negation operator

        SType,                     // simple type
        PType,                     // parameterized type
        IType,                     // intersection type
        UType,                     // union type

        Set,                       // value assignment
        Target,                    // assignment target
        InplaceAdd,                // inplace add assignment
        InplaceSub,                // inplace subtract assignment
        InplaceMul,                // inplace multiply assignment
        InplaceDiv,                // inplace divide assignment

        Deref,                     // dereference expression
        This,                      // resolve this reference
        Name,                      // resolve symbol
        Call,                      // call named function or method
        Keyword,                   // keyword argument

        New,                       // create object
        Build,                     // build object
        Data,                      // create data
        Lambda,                    // anonymous function

        Block,                     // list of forms evaluated in order
        If,                        // if statement
        Cond,                      // conditional expression
        Match,                     // match expression
        Case,                      // case clause
        While,                     // while statement
        For,                       // for statement
        Try,                       // try statement
        Return,                    // return immediately from call

        Val,                       // define val
        Var,                       // define var
        Unpack,                    // variable unpack
        Pack,                      // parameter pack
        Param,                     // function parameter
        Rest,                      // variadic parameter
        Ctx,                       // ctx parameter

        Generic,                   // generic type specification
        Placeholder,               // type placeholder
        Constraint,                // type constraint

        Def,                       // define function
        DefAlias,                  // define alias
        DefClass,                  // define class
        DefConcept,                // define concept
        DefEnum,                   // define enumeration
        DefInstance,               // define instance
        DefMagnet,                 // define magnet
        DefStruct,                 // define struct
        Namespace,                 // define namespace

        Super,                     // super statement
        Init,                      // init statement
        Impl,                      // impl statement

        ImportAll,                 // import all module symbols into environment
        ImportSymbols,             // import specified module symbols into environment
        ImportModule,              // import specified module symbols into environment
        ExportAll,                 // export all module symbols from environment
        ExportSymbols,             // export specified module symbols from environment
        ExportModule,              // export specified module symbols from environment
        Using,                     // insert impls into the current environment
        SymbolRef,                 // symbol reference

        LiteralValue,

        BaseEnum,
        NotationEnum,
        BindingEnum,
        MutationEnum,
        AccessEnum,
        BoundEnum,
        VarianceEnum,

        AssemblyLocation,
        SymbolPath,
        SymbolUrl,
        Identifier,
        Label,

        TypeOffset,
        DefaultOffset,
        FinallyOffset,
        RestOffset,
        GenericOffset,
        ImplementsOffset,

        NUM_IDS,                    // must be last
    };

    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstNilClass(
        &kLyricAstNs, LyricAstId::Nil, "Nil");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstUndefClass(
        &kLyricAstNs, LyricAstId::Undef, "Undef");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstFalseClass(
        &kLyricAstNs, LyricAstId::False, "False");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstTrueClass(
        &kLyricAstNs, LyricAstId::True, "True");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIntegerClass(
        &kLyricAstNs, LyricAstId::Integer, "Integer");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstFloatClass(
        &kLyricAstNs, LyricAstId::Float, "Float");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstCharClass(
        &kLyricAstNs, LyricAstId::Char, "Char");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstStringClass(
        &kLyricAstNs, LyricAstId::String, "String");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstUrlClass(
        &kLyricAstNs, LyricAstId::Url, "Url");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstPairClass(
        &kLyricAstNs, LyricAstId::Pair, "Pair");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstSeqClass(
        &kLyricAstNs, LyricAstId::Seq, "Seq");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstMapClass(
        &kLyricAstNs, LyricAstId::Map, "Map");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstRowClass(
        &kLyricAstNs, LyricAstId::Row, "Row");

    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstAddClass(
        &kLyricAstNs, LyricAstId::Add, "Add");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstSubClass(
        &kLyricAstNs, LyricAstId::Sub, "Sub");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstMulClass(
        &kLyricAstNs, LyricAstId::Mul, "Mul");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDivClass(
        &kLyricAstNs, LyricAstId::Div, "Div");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstNegClass(
        &kLyricAstNs, LyricAstId::Neg, "Neg");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIsEqClass(
        &kLyricAstNs, LyricAstId::IsEq, "IsEq");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIsLtClass(
        &kLyricAstNs, LyricAstId::IsLt, "IsLt");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIsLeClass(
        &kLyricAstNs, LyricAstId::IsLe, "IsLe");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIsGtClass(
        &kLyricAstNs, LyricAstId::IsGt, "IsGt");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIsGeClass(
        &kLyricAstNs, LyricAstId::IsGe, "IsGe");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIsAClass(
        &kLyricAstNs, LyricAstId::IsA, "IsA");

    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstAndClass(
        &kLyricAstNs, LyricAstId::And, "And");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstOrClass(
        &kLyricAstNs, LyricAstId::Or, "Or");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstNotClass(
        &kLyricAstNs, LyricAstId::Not, "Not");

    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstSTypeClass(
        &kLyricAstNs, LyricAstId::SType, "SType");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstPTypeClass(
        &kLyricAstNs, LyricAstId::PType, "PType");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstITypeClass(
        &kLyricAstNs, LyricAstId::IType, "IType");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstUTypeClass(
        &kLyricAstNs, LyricAstId::UType, "UType");

    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstSetClass(
        &kLyricAstNs, LyricAstId::Set, "Set");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstTargetClass(
        &kLyricAstNs, LyricAstId::Target, "Target");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstInplaceAddClass(
        &kLyricAstNs, LyricAstId::InplaceAdd, "InplaceAdd");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstInplaceSubClass(
        &kLyricAstNs, LyricAstId::InplaceSub, "InplaceSub");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstInplaceMulClass(
        &kLyricAstNs, LyricAstId::InplaceMul, "InplaceMul");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstInplaceDivClass(
        &kLyricAstNs, LyricAstId::InplaceDiv, "InplaceDiv");

    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDerefClass(
        &kLyricAstNs, LyricAstId::Deref, "Deref");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstThisClass(
        &kLyricAstNs, LyricAstId::This, "This");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstNameClass(
        &kLyricAstNs, LyricAstId::Name, "Name");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstCallClass(
        &kLyricAstNs, LyricAstId::Call, "Call");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstKeywordClass(
        &kLyricAstNs, LyricAstId::Keyword, "Keyword");

    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstNewClass(
        &kLyricAstNs, LyricAstId::New, "New");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstBuildClass(
        &kLyricAstNs, LyricAstId::Build, "Build");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDataClass(
        &kLyricAstNs, LyricAstId::Data, "Data");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstLambdaClass(
        &kLyricAstNs, LyricAstId::Lambda, "Lambda");

    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstBlockClass(
        &kLyricAstNs, LyricAstId::Block, "Block");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIfClass(
        &kLyricAstNs, LyricAstId::If, "If");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstCondClass(
        &kLyricAstNs, LyricAstId::Cond, "Cond");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstMatchClass(
        &kLyricAstNs, LyricAstId::Match, "Match");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstCaseClass(
        &kLyricAstNs, LyricAstId::Case, "Case");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstWhileClass(
        &kLyricAstNs, LyricAstId::While, "While");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstForClass(
        &kLyricAstNs, LyricAstId::For, "For");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstTryClass(
        &kLyricAstNs, LyricAstId::Try, "Try");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstReturnClass(
        &kLyricAstNs, LyricAstId::Return, "Return");

    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstValClass(
        &kLyricAstNs, LyricAstId::Val, "Val");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstVarClass(
        &kLyricAstNs, LyricAstId::Var, "Var");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstUnpackClass(
        &kLyricAstNs, LyricAstId::Unpack, "Unpack");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstPackClass(
        &kLyricAstNs, LyricAstId::Pack, "Pack");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstParamClass(
        &kLyricAstNs, LyricAstId::Param, "Param");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstRestClass(
        &kLyricAstNs, LyricAstId::Rest, "Rest");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstCtxClass(
        &kLyricAstNs, LyricAstId::Ctx, "Ctx");

    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstGenericClass(
        &kLyricAstNs, LyricAstId::Generic, "Generic");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstPlaceholderClass(
        &kLyricAstNs, LyricAstId::Placeholder, "Placeholder");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstConstraintClass(
        &kLyricAstNs, LyricAstId::Constraint, "Constraint");

    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefClass(
        &kLyricAstNs, LyricAstId::Def, "Def");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefAliasClass(
        &kLyricAstNs, LyricAstId::DefAlias, "DefAlias");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefClassClass(
        &kLyricAstNs, LyricAstId::DefClass, "DefClass");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefConceptClass(
        &kLyricAstNs, LyricAstId::DefConcept, "DefConcept");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefEnumClass(
        &kLyricAstNs, LyricAstId::DefEnum, "DefEnum");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefInstanceClass(
        &kLyricAstNs, LyricAstId::DefInstance, "DefInstance");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefMagnetClass(
        &kLyricAstNs, LyricAstId::DefMagnet, "DefMagnet");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefStructClass(
        &kLyricAstNs, LyricAstId::DefStruct, "DefStruct");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstNamespaceClass(
        &kLyricAstNs, LyricAstId::Namespace, "Namespace");

    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstSuperClass(
        &kLyricAstNs, LyricAstId::Super, "Super");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstInitClass(
        &kLyricAstNs, LyricAstId::Init, "Init");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstImplClass(
        &kLyricAstNs, LyricAstId::Impl, "Impl");

    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstImportAllClass(
        &kLyricAstNs, LyricAstId::ImportAll, "ImportAll");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstImportSymbolsClass(
        &kLyricAstNs, LyricAstId::ImportSymbols, "ImportSymbols");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstImportModuleClass(
        &kLyricAstNs, LyricAstId::ImportModule, "ImportModule");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstExportAllClass(
        &kLyricAstNs, LyricAstId::ExportAll, "ExportAll");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstExportSymbolsClass(
        &kLyricAstNs, LyricAstId::ExportSymbols, "ExportSymbols");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstExportModuleClass(
        &kLyricAstNs, LyricAstId::ExportModule, "ExportModule");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstUsingClass(
        &kLyricAstNs, LyricAstId::Using, "Using");
    constexpr tempo_utils::SchemaClass<LyricAstNs,LyricAstId> kLyricAstSymbolRefClass(
        &kLyricAstNs, LyricAstId::SymbolRef, "SymbolRef");

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstLiteralValueProperty(
        &kLyricAstNs, LyricAstId::LiteralValue, "LiteralValue", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstBaseEnumProperty(
        &kLyricAstNs, LyricAstId::BaseEnum, "BaseEnum", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstNotationEnumProperty(
        &kLyricAstNs, LyricAstId::NotationEnum, "NotationEnum", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstBindingEnumProperty(
        &kLyricAstNs, LyricAstId::BindingEnum, "BindingEnum", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstMutationEnumProperty(
        &kLyricAstNs, LyricAstId::MutationEnum, "MutationEnum", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstAccessEnumProperty(
        &kLyricAstNs, LyricAstId::AccessEnum, "AccessEnum", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstBoundEnumProperty(
        &kLyricAstNs, LyricAstId::BoundEnum, "BoundEnum", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstVarianceEnumProperty(
        &kLyricAstNs, LyricAstId::VarianceEnum, "VarianceEnum", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstAssemblyLocationProperty(
        &kLyricAstNs, LyricAstId::AssemblyLocation, "AssemblyLocation", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstSymbolPathProperty(
        &kLyricAstNs, LyricAstId::SymbolPath, "SymbolPath", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstSymbolUrlProperty(
        &kLyricAstNs, LyricAstId::SymbolUrl, "SymbolUrl", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstIdentifierProperty(
        &kLyricAstNs, LyricAstId::Identifier, "Identifier", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstLabelProperty(
        &kLyricAstNs, LyricAstId::Label, "Label", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstTypeOffsetProperty(
        &kLyricAstNs, LyricAstId::TypeOffset, "TypeOffset", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstDefaultOffsetProperty(
        &kLyricAstNs, LyricAstId::DefaultOffset, "DefaultOffset", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstFinallyOffsetProperty(
        &kLyricAstNs, LyricAstId::FinallyOffset, "FinallyOffset", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstRestOffsetProperty(
        &kLyricAstNs, LyricAstId::RestOffset, "RestOffset", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstGenericOffsetProperty(
        &kLyricAstNs, LyricAstId::GenericOffset, "GenericOffset", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstImplementsOffsetProperty(
        &kLyricAstNs, LyricAstId::ImplementsOffset, "ImplementsOffset", tempo_utils::PropertyType::kUInt32);

    constexpr std::array<
        const tempo_utils::SchemaResource<LyricAstNs,LyricAstId> *,
        static_cast<std::size_t>(LyricAstId::NUM_IDS)>
    kLyricAstResources = {

        &kLyricAstNilClass,                       // nil literal
        &kLyricAstUndefClass,                     // undef literal
        &kLyricAstFalseClass,                     // false literal
        &kLyricAstTrueClass,                      // true literal
        &kLyricAstIntegerClass,                   // integer literal
        &kLyricAstFloatClass,                     // float literal
        &kLyricAstCharClass,                      // char literal
        &kLyricAstStringClass,                    // string literal
        &kLyricAstUrlClass,                       // url literal

        &kLyricAstPairClass,                      // pair container
        &kLyricAstSeqClass,                       // seq container
        &kLyricAstMapClass,                       // map container
        &kLyricAstRowClass,                       // row container

        &kLyricAstAddClass,                       // addition operator
        &kLyricAstSubClass,                       // subtraction operator
        &kLyricAstMulClass,                       // multiplication operator
        &kLyricAstDivClass,                       // division operator
        &kLyricAstNegClass,                       // additive inverse operator
        &kLyricAstIsEqClass,                      // equals operator
        &kLyricAstIsLtClass,                      // less-than operator
        &kLyricAstIsLeClass,                      // less-than-or-equals operator
        &kLyricAstIsGtClass,                      // greater-than operator
        &kLyricAstIsGeClass,                      // greater-than-or-equals operator
        &kLyricAstIsAClass,                       // type-equals operator

        &kLyricAstAndClass,                       // logical conjunction operator
        &kLyricAstOrClass,                        // logical disjunction operator
        &kLyricAstNotClass,                       // logical negation operator

        &kLyricAstSTypeClass,                     // simple type
        &kLyricAstPTypeClass,                     // parameterized type
        &kLyricAstITypeClass,                     // intersection type
        &kLyricAstUTypeClass,                     // union type

        &kLyricAstSetClass,                       // value assignment
        &kLyricAstTargetClass,                    // assignment target
        &kLyricAstInplaceAddClass,                // inplace add assignment
        &kLyricAstInplaceSubClass,                // inplace subtract assignment
        &kLyricAstInplaceMulClass,                // inplace multiply assignment
        &kLyricAstInplaceDivClass,                // inplace divide assignment

        &kLyricAstDerefClass,                     // dereference expression
        &kLyricAstThisClass,                      // resolve this reference
        &kLyricAstNameClass,                      // resolve symbol
        &kLyricAstCallClass,                      // call named function or method
        &kLyricAstKeywordClass,                   // keyword argument

        &kLyricAstNewClass,                       // create object
        &kLyricAstBuildClass,                     // build object
        &kLyricAstDataClass,                      // create data
        &kLyricAstLambdaClass,                    // anonymous function

        &kLyricAstBlockClass,                     // list of forms evaluated in order
        &kLyricAstIfClass,                        // if statement
        &kLyricAstCondClass,                      // conditional expression
        &kLyricAstMatchClass,                     // match expression
        &kLyricAstCaseClass,                      // case clause
        &kLyricAstWhileClass,                     // while statement
        &kLyricAstForClass,                       // for statement
        &kLyricAstTryClass,                       // try statement
        &kLyricAstReturnClass,                    // return immediately from call

        &kLyricAstValClass,                       // define val
        &kLyricAstVarClass,                       // define var
        &kLyricAstUnpackClass,                    // variable unpack
        &kLyricAstPackClass,                      // parameter pack
        &kLyricAstParamClass,                     // function parameter
        &kLyricAstRestClass,                      // variadic parameter
        &kLyricAstCtxClass,                       // ctx parameter

        &kLyricAstGenericClass,                   // generic type specification
        &kLyricAstPlaceholderClass,               // type placeholder
        &kLyricAstConstraintClass,                // type constraint

        &kLyricAstDefClass,                       // define function
        &kLyricAstDefAliasClass,                  // define alias
        &kLyricAstDefClassClass,                  // define class
        &kLyricAstDefConceptClass,                // define concept
        &kLyricAstDefEnumClass,                   // define enumeration
        &kLyricAstDefInstanceClass,               // define instance
        &kLyricAstDefMagnetClass,                 // define magnet
        &kLyricAstDefStructClass,                 // define struct
        &kLyricAstNamespaceClass,                 // define namespace

        &kLyricAstSuperClass,                     // super statement
        &kLyricAstInitClass,                      // init statement
        &kLyricAstImplClass,                      // impl statement

        &kLyricAstImportAllClass,                 // import all module symbols into environment
        &kLyricAstImportSymbolsClass,             // import specified module symbols into environment
        &kLyricAstImportModuleClass,              // import specified module symbols into environment
        &kLyricAstExportAllClass,                 // export all module symbols from environment
        &kLyricAstExportSymbolsClass,             // export specified module symbols from environment
        &kLyricAstExportModuleClass,              // export specified module symbols from environment
        &kLyricAstUsingClass,                     // insert impls into the current environment
        &kLyricAstSymbolRefClass,                 // symbol reference

        &kLyricAstLiteralValueProperty,

        &kLyricAstBaseEnumProperty,
        &kLyricAstNotationEnumProperty,
        &kLyricAstBindingEnumProperty,
        &kLyricAstMutationEnumProperty,
        &kLyricAstAccessEnumProperty,
        &kLyricAstBoundEnumProperty,
        &kLyricAstVarianceEnumProperty,

        &kLyricAstAssemblyLocationProperty,
        &kLyricAstSymbolPathProperty,
        &kLyricAstSymbolUrlProperty,
        &kLyricAstIdentifierProperty,
        &kLyricAstLabelProperty,

        &kLyricAstTypeOffsetProperty,
        &kLyricAstDefaultOffsetProperty,
        &kLyricAstFinallyOffsetProperty,
        &kLyricAstRestOffsetProperty,
        &kLyricAstGenericOffsetProperty,
        &kLyricAstImplementsOffsetProperty,
    };

    constexpr tempo_utils::SchemaVocabulary<LyricAstNs, LyricAstId>
    kLyricAstVocabulary(&kLyricAstNs, &kLyricAstResources);
}

#endif // LYRIC_SCHEMA_AST_SCHEMA_H