#ifndef LYRIC_SCHEMA_AST_SCHEMA_H
#define LYRIC_SCHEMA_AST_SCHEMA_H

#include <array>
#include <tempo_schema/schema.h>

#include <tempo_schema/schema_namespace.h>

namespace lyric_schema {

    class LyricAstNs : public tempo_schema::SchemaNs {
    public:
        constexpr LyricAstNs() : tempo_schema::SchemaNs("dev.zuri.ns:ast-1") {};
    };
    constexpr LyricAstNs kLyricAstNs;

    enum class LyricAstId {

        // AST classes

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

        And,                       // logical conjunction operator
        Or,                        // logical disjunction operator
        Not,                       // logical negation operator

        SType,                     // simple type
        PType,                     // parameterized type
        IType,                     // intersection type
        UType,                     // union type
        XType,                     // NoReturn type
        TypeArguments,             // type arguments
        TypeOf,                    // typeof expression
        TypeName,                  // declare type

        Set,                       // value assignment
        Target,                    // assignment target
        InplaceAdd,                // inplace add assignment
        InplaceSub,                // inplace subtract assignment
        InplaceMul,                // inplace multiply assignment
        InplaceDiv,                // inplace divide assignment

        DataDeref,                 // data dereference expression
        SymbolDeref,               // symbol dereference expression
        This,                      // resolve this reference
        Name,                      // resolve symbol
        Call,                      // call named function or method
        Keyword,                   // keyword argument

        New,                       // create object
        Lambda,                    // anonymous function
        LambdaFrom,                // anonymous function from an existing function

        Block,                     // list of forms evaluated in order
        If,                        // if statement
        Cond,                      // conditional expression
        Match,                     // match expression
        When,                      // when clause
        While,                     // while statement
        For,                       // for statement
        Try,                       // try statement
        Return,                    // return immediately from call
        Expect,                    // return immediately if expression returns status
        Raise,                     // raise status as exception if expression returns status

        Val,                       // define val
        Var,                       // define var
        Case,                      // define case
        Unpack,                    // variable unpack
        Pack,                      // parameter pack
        Param,                     // function parameter
        Rest,                      // variadic parameter
        Ctx,                       // ctx parameter

        Generic,                   // generic type specification
        Placeholder,               // type placeholder
        Constraint,                // type constraint

        Decl,                      // declare action
        Def,                       // define function
        DefAlias,                  // define alias
        DefClass,                  // define class
        DefConcept,                // define concept
        DefEnum,                   // define enumeration
        DefInstance,               // define instance
        DefStruct,                 // define struct
        DefStatic,                 // define static
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

        MacroCall,                 // macro invocation
        MacroList,                 // sequence of macro invocations
        Pragma,                    // compiler pragma

        // AST properties

        LiteralValue,

        BaseEnum,
        NotationEnum,
        BoundEnum,
        VarianceEnum,
        DeriveEnum,

        ImportLocation,
        ModuleLocation,
        SymbolPath,
        SymbolUrl,
        Identifier,
        Label,
        IsHidden,
        IsVariable,

        TypeOffset,
        DefaultOffset,
        FinallyOffset,
        RestOffset,
        GenericOffset,
        TypeArgumentsOffset,
        MacroListOffset,

        NUM_IDS,                    // must be last
    };

    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstNilClass(
        &kLyricAstNs, LyricAstId::Nil, "Nil");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstUndefClass(
        &kLyricAstNs, LyricAstId::Undef, "Undef");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstFalseClass(
        &kLyricAstNs, LyricAstId::False, "False");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstTrueClass(
        &kLyricAstNs, LyricAstId::True, "True");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIntegerClass(
        &kLyricAstNs, LyricAstId::Integer, "Integer");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstFloatClass(
        &kLyricAstNs, LyricAstId::Float, "Float");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstCharClass(
        &kLyricAstNs, LyricAstId::Char, "Char");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstStringClass(
        &kLyricAstNs, LyricAstId::String, "String");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstUrlClass(
        &kLyricAstNs, LyricAstId::Url, "Url");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstPairClass(
        &kLyricAstNs, LyricAstId::Pair, "Pair");

    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstAddClass(
        &kLyricAstNs, LyricAstId::Add, "Add");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstSubClass(
        &kLyricAstNs, LyricAstId::Sub, "Sub");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstMulClass(
        &kLyricAstNs, LyricAstId::Mul, "Mul");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDivClass(
        &kLyricAstNs, LyricAstId::Div, "Div");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstNegClass(
        &kLyricAstNs, LyricAstId::Neg, "Neg");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIsEqClass(
        &kLyricAstNs, LyricAstId::IsEq, "IsEq");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIsLtClass(
        &kLyricAstNs, LyricAstId::IsLt, "IsLt");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIsLeClass(
        &kLyricAstNs, LyricAstId::IsLe, "IsLe");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIsGtClass(
        &kLyricAstNs, LyricAstId::IsGt, "IsGt");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIsGeClass(
        &kLyricAstNs, LyricAstId::IsGe, "IsGe");

    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstAndClass(
        &kLyricAstNs, LyricAstId::And, "And");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstOrClass(
        &kLyricAstNs, LyricAstId::Or, "Or");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstNotClass(
        &kLyricAstNs, LyricAstId::Not, "Not");

    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstSTypeClass(
        &kLyricAstNs, LyricAstId::SType, "SType");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstPTypeClass(
        &kLyricAstNs, LyricAstId::PType, "PType");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstITypeClass(
        &kLyricAstNs, LyricAstId::IType, "IType");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstUTypeClass(
        &kLyricAstNs, LyricAstId::UType, "UType");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstXTypeClass(
        &kLyricAstNs, LyricAstId::XType, "XType");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstTypeArgumentsClass(
        &kLyricAstNs, LyricAstId::TypeArguments, "TypeArguments");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstTypeOfClass(
        &kLyricAstNs, LyricAstId::TypeOf, "TypeOf");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstTypeNameClass(
        &kLyricAstNs, LyricAstId::TypeName, "TypeName");

    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstSetClass(
        &kLyricAstNs, LyricAstId::Set, "Set");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstTargetClass(
        &kLyricAstNs, LyricAstId::Target, "Target");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstInplaceAddClass(
        &kLyricAstNs, LyricAstId::InplaceAdd, "InplaceAdd");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstInplaceSubClass(
        &kLyricAstNs, LyricAstId::InplaceSub, "InplaceSub");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstInplaceMulClass(
        &kLyricAstNs, LyricAstId::InplaceMul, "InplaceMul");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstInplaceDivClass(
        &kLyricAstNs, LyricAstId::InplaceDiv, "InplaceDiv");

    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDataDerefClass(
        &kLyricAstNs, LyricAstId::DataDeref, "DataDeref");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstSymbolDerefClass(
        &kLyricAstNs, LyricAstId::SymbolDeref, "SymbolDeref");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstThisClass(
        &kLyricAstNs, LyricAstId::This, "This");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstNameClass(
        &kLyricAstNs, LyricAstId::Name, "Name");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstCallClass(
        &kLyricAstNs, LyricAstId::Call, "Call");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstKeywordClass(
        &kLyricAstNs, LyricAstId::Keyword, "Keyword");

    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstNewClass(
        &kLyricAstNs, LyricAstId::New, "New");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstLambdaClass(
        &kLyricAstNs, LyricAstId::Lambda, "Lambda");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstLambdaFromClass(
        &kLyricAstNs, LyricAstId::LambdaFrom, "LambdaFrom");

    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstBlockClass(
        &kLyricAstNs, LyricAstId::Block, "Block");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstIfClass(
        &kLyricAstNs, LyricAstId::If, "If");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstCondClass(
        &kLyricAstNs, LyricAstId::Cond, "Cond");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstMatchClass(
        &kLyricAstNs, LyricAstId::Match, "Match");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstWhenClass(
        &kLyricAstNs, LyricAstId::When, "When");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstWhileClass(
        &kLyricAstNs, LyricAstId::While, "While");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstForClass(
        &kLyricAstNs, LyricAstId::For, "For");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstTryClass(
        &kLyricAstNs, LyricAstId::Try, "Try");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstReturnClass(
        &kLyricAstNs, LyricAstId::Return, "Return");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstExpectClass(
        &kLyricAstNs, LyricAstId::Expect, "Expect");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstRaiseClass(
        &kLyricAstNs, LyricAstId::Raise, "Raise");

    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstValClass(
        &kLyricAstNs, LyricAstId::Val, "Val");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstVarClass(
        &kLyricAstNs, LyricAstId::Var, "Var");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstCaseClass(
        &kLyricAstNs, LyricAstId::Case, "Case");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstUnpackClass(
        &kLyricAstNs, LyricAstId::Unpack, "Unpack");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstPackClass(
        &kLyricAstNs, LyricAstId::Pack, "Pack");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstParamClass(
        &kLyricAstNs, LyricAstId::Param, "Param");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstRestClass(
        &kLyricAstNs, LyricAstId::Rest, "Rest");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstCtxClass(
        &kLyricAstNs, LyricAstId::Ctx, "Ctx");

    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstGenericClass(
        &kLyricAstNs, LyricAstId::Generic, "Generic");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstPlaceholderClass(
        &kLyricAstNs, LyricAstId::Placeholder, "Placeholder");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstConstraintClass(
        &kLyricAstNs, LyricAstId::Constraint, "Constraint");

    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDeclClass(
        &kLyricAstNs, LyricAstId::Decl, "Decl");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefClass(
        &kLyricAstNs, LyricAstId::Def, "Def");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefAliasClass(
        &kLyricAstNs, LyricAstId::DefAlias, "DefAlias");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefClassClass(
        &kLyricAstNs, LyricAstId::DefClass, "DefClass");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefConceptClass(
        &kLyricAstNs, LyricAstId::DefConcept, "DefConcept");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefEnumClass(
        &kLyricAstNs, LyricAstId::DefEnum, "DefEnum");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefInstanceClass(
        &kLyricAstNs, LyricAstId::DefInstance, "DefInstance");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefStructClass(
        &kLyricAstNs, LyricAstId::DefStruct, "DefStruct");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstDefStaticClass(
        &kLyricAstNs, LyricAstId::DefStatic, "DefStatic");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstNamespaceClass(
        &kLyricAstNs, LyricAstId::Namespace, "Namespace");

    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstSuperClass(
        &kLyricAstNs, LyricAstId::Super, "Super");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstInitClass(
        &kLyricAstNs, LyricAstId::Init, "Init");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstImplClass(
        &kLyricAstNs, LyricAstId::Impl, "Impl");

    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstImportAllClass(
        &kLyricAstNs, LyricAstId::ImportAll, "ImportAll");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstImportSymbolsClass(
        &kLyricAstNs, LyricAstId::ImportSymbols, "ImportSymbols");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstImportModuleClass(
        &kLyricAstNs, LyricAstId::ImportModule, "ImportModule");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstExportAllClass(
        &kLyricAstNs, LyricAstId::ExportAll, "ExportAll");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstExportSymbolsClass(
        &kLyricAstNs, LyricAstId::ExportSymbols, "ExportSymbols");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstExportModuleClass(
        &kLyricAstNs, LyricAstId::ExportModule, "ExportModule");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstUsingClass(
        &kLyricAstNs, LyricAstId::Using, "Using");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstSymbolRefClass(
        &kLyricAstNs, LyricAstId::SymbolRef, "SymbolRef");

    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstMacroCallClass(
        &kLyricAstNs, LyricAstId::MacroCall, "MacroCall");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstMacroListClass(
        &kLyricAstNs, LyricAstId::MacroList, "MacroList");
    constexpr tempo_schema::SchemaClass<LyricAstNs,LyricAstId> kLyricAstPragmaClass(
        &kLyricAstNs, LyricAstId::Pragma, "Pragma");

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstLiteralValueProperty(
        &kLyricAstNs, LyricAstId::LiteralValue, "LiteralValue", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstBaseEnumProperty(
        &kLyricAstNs, LyricAstId::BaseEnum, "BaseEnum", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstNotationEnumProperty(
        &kLyricAstNs, LyricAstId::NotationEnum, "NotationEnum", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstBoundEnumProperty(
        &kLyricAstNs, LyricAstId::BoundEnum, "BoundEnum", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstVarianceEnumProperty(
        &kLyricAstNs, LyricAstId::VarianceEnum, "VarianceEnum", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
        kLyricAstDeriveEnumProperty(
        &kLyricAstNs, LyricAstId::DeriveEnum, "DeriveEnum", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstImportLocationProperty(
        &kLyricAstNs, LyricAstId::ImportLocation, "ImportLocation", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstModuleLocationProperty(
        &kLyricAstNs, LyricAstId::ModuleLocation, "ModuleLocation", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstSymbolPathProperty(
        &kLyricAstNs, LyricAstId::SymbolPath, "SymbolPath", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstSymbolUrlProperty(
        &kLyricAstNs, LyricAstId::SymbolUrl, "SymbolUrl", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstIdentifierProperty(
        &kLyricAstNs, LyricAstId::Identifier, "Identifier", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstLabelProperty(
        &kLyricAstNs, LyricAstId::Label, "Label", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstIsHiddenProperty(
        &kLyricAstNs, LyricAstId::IsHidden, "IsHidden", tempo_schema::PropertyType::kBool);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
        kLyricAstIsVariableProperty(
        &kLyricAstNs, LyricAstId::IsVariable, "IsVariable", tempo_schema::PropertyType::kBool);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstTypeOffsetProperty(
        &kLyricAstNs, LyricAstId::TypeOffset, "TypeOffset", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstDefaultOffsetProperty(
        &kLyricAstNs, LyricAstId::DefaultOffset, "DefaultOffset", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstFinallyOffsetProperty(
        &kLyricAstNs, LyricAstId::FinallyOffset, "FinallyOffset", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstRestOffsetProperty(
        &kLyricAstNs, LyricAstId::RestOffset, "RestOffset", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstGenericOffsetProperty(
        &kLyricAstNs, LyricAstId::GenericOffset, "GenericOffset", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstTypeArgumentsOffsetProperty(
        &kLyricAstNs, LyricAstId::TypeArgumentsOffset, "TypeArgumentsOffset", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricAstNs,LyricAstId>
    kLyricAstMacroListOffsetProperty(
        &kLyricAstNs, LyricAstId::MacroListOffset, "MacroListOffset", tempo_schema::PropertyType::kUInt32);

    constexpr std::array<
        const tempo_schema::SchemaResource<LyricAstNs,LyricAstId> *,
        static_cast<std::size_t>(LyricAstId::NUM_IDS)>
    kLyricAstResources = {

        &kLyricAstNilClass,
        &kLyricAstUndefClass,
        &kLyricAstFalseClass,
        &kLyricAstTrueClass,
        &kLyricAstIntegerClass,
        &kLyricAstFloatClass,
        &kLyricAstCharClass,
        &kLyricAstStringClass,
        &kLyricAstUrlClass,
        &kLyricAstPairClass,

        &kLyricAstAddClass,
        &kLyricAstSubClass,
        &kLyricAstMulClass,
        &kLyricAstDivClass,
        &kLyricAstNegClass,
        &kLyricAstIsEqClass,
        &kLyricAstIsLtClass,
        &kLyricAstIsLeClass,
        &kLyricAstIsGtClass,
        &kLyricAstIsGeClass,

        &kLyricAstAndClass,
        &kLyricAstOrClass,
        &kLyricAstNotClass,

        &kLyricAstSTypeClass,
        &kLyricAstPTypeClass,
        &kLyricAstITypeClass,
        &kLyricAstUTypeClass,
        &kLyricAstXTypeClass,
        &kLyricAstTypeArgumentsClass,
        &kLyricAstTypeOfClass,
        &kLyricAstTypeNameClass,

        &kLyricAstSetClass,
        &kLyricAstTargetClass,
        &kLyricAstInplaceAddClass,
        &kLyricAstInplaceSubClass,
        &kLyricAstInplaceMulClass,
        &kLyricAstInplaceDivClass,

        &kLyricAstDataDerefClass,
        &kLyricAstSymbolDerefClass,
        &kLyricAstThisClass,
        &kLyricAstNameClass,
        &kLyricAstCallClass,
        &kLyricAstKeywordClass,

        &kLyricAstNewClass,
        &kLyricAstLambdaClass,
        &kLyricAstLambdaFromClass,

        &kLyricAstBlockClass,
        &kLyricAstIfClass,
        &kLyricAstCondClass,
        &kLyricAstMatchClass,
        &kLyricAstWhenClass,
        &kLyricAstWhileClass,
        &kLyricAstForClass,
        &kLyricAstTryClass,
        &kLyricAstReturnClass,
        &kLyricAstExpectClass,
        &kLyricAstRaiseClass,

        &kLyricAstValClass,
        &kLyricAstVarClass,
        &kLyricAstCaseClass,
        &kLyricAstUnpackClass,
        &kLyricAstPackClass,
        &kLyricAstParamClass,
        &kLyricAstRestClass,
        &kLyricAstCtxClass,

        &kLyricAstGenericClass,
        &kLyricAstPlaceholderClass,
        &kLyricAstConstraintClass,

        &kLyricAstDeclClass,
        &kLyricAstDefClass,
        &kLyricAstDefAliasClass,
        &kLyricAstDefClassClass,
        &kLyricAstDefConceptClass,
        &kLyricAstDefEnumClass,
        &kLyricAstDefInstanceClass,
        &kLyricAstDefStructClass,
        &kLyricAstDefStaticClass,
        &kLyricAstNamespaceClass,

        &kLyricAstSuperClass,
        &kLyricAstInitClass,
        &kLyricAstImplClass,

        &kLyricAstImportAllClass,
        &kLyricAstImportSymbolsClass,
        &kLyricAstImportModuleClass,
        &kLyricAstExportAllClass,
        &kLyricAstExportSymbolsClass,
        &kLyricAstExportModuleClass,
        &kLyricAstUsingClass,
        &kLyricAstSymbolRefClass,

        &kLyricAstMacroCallClass,
        &kLyricAstMacroListClass,
        &kLyricAstPragmaClass,

        &kLyricAstLiteralValueProperty,

        &kLyricAstBaseEnumProperty,
        &kLyricAstNotationEnumProperty,
        &kLyricAstBoundEnumProperty,
        &kLyricAstVarianceEnumProperty,
        &kLyricAstDeriveEnumProperty,

        &kLyricAstImportLocationProperty,
        &kLyricAstModuleLocationProperty,
        &kLyricAstSymbolPathProperty,
        &kLyricAstSymbolUrlProperty,
        &kLyricAstIdentifierProperty,
        &kLyricAstLabelProperty,
        &kLyricAstIsHiddenProperty,
        &kLyricAstIsVariableProperty,

        &kLyricAstTypeOffsetProperty,
        &kLyricAstDefaultOffsetProperty,
        &kLyricAstFinallyOffsetProperty,
        &kLyricAstRestOffsetProperty,
        &kLyricAstGenericOffsetProperty,
        &kLyricAstTypeArgumentsOffsetProperty,
        &kLyricAstMacroListOffsetProperty,
    };

    constexpr tempo_schema::SchemaVocabulary<LyricAstNs, LyricAstId>
    kLyricAstVocabulary(&kLyricAstNs, &kLyricAstResources);
}

#endif // LYRIC_SCHEMA_AST_SCHEMA_H