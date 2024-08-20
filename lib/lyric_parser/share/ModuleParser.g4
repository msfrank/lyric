parser grammar ModuleParser;

options { tokenVocab = ModuleLexer; }


root                : block ;

block               : form+ ;
form                : statement | expression | macro ;

expression          : basicExpression
                    | lambdaExpression
                    | ifThenElseExpression
                    | matchExpression
                    | condExpression
                    ;

statement           : valStatement
                    | varStatement
                    | defStatement
                    | defaliasStatement
                    | defclassStatement
                    | defconceptStatement
                    | defenumStatement
                    | definstanceStatement
                    | defstructStatement
                    | globalStatement
                    | namespaceStatement
                    | setStatement
                    | ifStatement
                    | condIfStatement
                    | whileStatement
                    | forStatement
                    | tryStatement
                    | returnStatement
                    | importStatement
                    | exportStatement
                    | usingStatement
                    ;


// symbols

symbolIdentifier    : Identifier ;
symbolPath          : Identifier ( DotOperator Identifier )? ;
symbolAlias         : NamedKeyword Identifier ;


// types

simpleType          : Identifier ( DotOperator Identifier )* ;
parametricType      : simpleType BracketOpen assignableType ( CommaOperator assignableType )* BracketClose ;
singularType        : parametricType | simpleType ;
unionType           : singularType ( PipeOperator singularType )+ ;
intersectionType    : singularType ( AmpersandOperator singularType )+ ;

assignableType      : singularType | unionType | intersectionType ;

typeArguments       : BracketOpen assignableType ( CommaOperator assignableType )* BracketClose ;


// placeholder

covariantPlaceholder        : PlusOperator Identifier ;
contravariantPlaceholder    : MinusOperator Identifier ;
invariantPlaceholder        : Identifier ;
placeholder                 : covariantPlaceholder | contravariantPlaceholder | invariantPlaceholder ;
placeholderSpec             : BracketOpen placeholder ( CommaOperator placeholder )* BracketClose ;


// constraint

upperTypeBound      : Identifier IsLtOperator assignableType ;
lowerTypeBound      : Identifier IsGtOperator assignableType ;
constraint          : upperTypeBound | lowerTypeBound ;
constraintSpec      : WhereKeyword constraint ( CommaOperator constraint )* ;


// argument lists

argSpec             : Identifier AssignOperator expression | expression ;
argList             : argSpec ( CommaOperator argSpec )* ;


// initializer

defaultInitializer  : assignableType CurlyOpen argList? CurlyClose                      # defaultInitializerTypedNew
                    | CurlyOpen argList? CurlyClose                                     # defaultInitializerNew
                    | literal                                                           # defaultInitializerLiteral
                    ;


// parameter lists

paramType           : ColonOperator assignableType ;
paramDefault        : AssignOperator defaultInitializer ;
positionalParam     : VarKeyword? Identifier paramType paramDefault? ;
namedParam          : VarKeyword? NamedKeyword Identifier paramType paramDefault? ;
renamedParam        : VarKeyword? Identifier NamedKeyword Identifier paramType paramDefault? ;
namedCtx            : UsingKeyword Identifier paramType ;
renamedCtx          : Identifier UsingKeyword Identifier paramType ;
param               : positionalParam | namedParam | renamedParam | namedCtx | renamedCtx ;
restParam           : VarKeyword? Identifier paramType ;
rest                : EllipsisOperator restParam? ;

paramList           : rest | param ( CommaOperator param )* ( CommaOperator rest )? ;
paramSpec           : ParenOpen paramList? ParenClose ;
returnSpec          : ColonOperator assignableType ;


// val statement

valStatement        : ValKeyword symbolIdentifier ColonOperator assignableType
                        AssignOperator expression                                       # typedVal
                    | ValKeyword symbolIdentifier AssignOperator expression             # untypedVal
                    ;


// var statement

varStatement        : VarKeyword symbolIdentifier ColonOperator assignableType
                        AssignOperator expression                                       # typedVar
                    | VarKeyword symbolIdentifier AssignOperator expression             # untypedVar
                    ;


// set statement

assignmentSpec      : ( ThisKeyword | Identifier ) ( DotOperator Identifier )+          # memberAssignment
                    | Identifier                                                        # nameAssignment
                    ;

assignmentOp        : AssignOperator
                    | PlusAssignOperator
                    | MinusAssignOperator
                    | StarAssignOperator
                    | SlashAssignOperator
                    ;
setStatement        : SetKeyword assignmentSpec assignmentOp expression ;


// def statement

defStatement        : DefKeyword symbolIdentifier
                        placeholderSpec? paramSpec returnSpec constraintSpec?
                        CurlyOpen block CurlyClose ;

// impl statement

implDef             : DefKeyword symbolIdentifier paramSpec returnSpec CurlyOpen block CurlyClose ;
implSpec            : implDef ;


// defclass statement

classSuper          : FromKeyword assignableType ParenOpen argList? ParenClose ;
classImpl           : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
classVal            : ValKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator defaultInitializer )? ;
classVar            : VarKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator defaultInitializer )? ;
classDef            : DefKeyword symbolIdentifier
                        placeholderSpec? paramSpec returnSpec constraintSpec?
                        CurlyOpen block CurlyClose ;
classInit           : InitKeyword paramSpec classSuper? CurlyOpen block? CurlyClose ;
genericClass        : placeholderSpec constraintSpec? ;
classDerives        : ( SealedKeyword | FinalKeyword ) ;
classSpec           : classInit | classVal | classVar | classDef | classImpl ;
defclassStatement   : DefClassKeyword symbolIdentifier genericClass? classDerives? CurlyOpen classSpec*  CurlyClose ;


// defconcept statement

conceptDecl         : DeclKeyword symbolIdentifier paramSpec returnSpec ;
conceptImpl         : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
genericConcept      : placeholderSpec constraintSpec? ;
conceptDerives      : ( SealedKeyword | FinalKeyword ) ;
conceptSpec         : conceptDecl | conceptImpl ;
defconceptStatement : DefConceptKeyword symbolIdentifier genericConcept? conceptDerives? CurlyOpen conceptSpec* CurlyClose ;


// definstance statement

instanceVal         : ValKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator defaultInitializer )? ;
instanceVar         : VarKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator defaultInitializer )? ;
instanceDef         : DefKeyword symbolIdentifier paramSpec returnSpec CurlyOpen block CurlyClose ;
instanceImpl        : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
instanceSpec        : instanceVal | instanceVar | instanceDef | instanceImpl ;
definstanceStatement: DefInstanceKeyword symbolIdentifier CurlyOpen instanceSpec* CurlyClose ;


// defenum statement

enumInit            : InitKeyword paramSpec CurlyOpen block? CurlyClose ;
enumVal             : ValKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator defaultInitializer )? ;
enumDef             : DefKeyword symbolIdentifier paramSpec returnSpec CurlyOpen block CurlyClose ;
enumCase            : CaseKeyword symbolIdentifier ( ParenOpen argList? ParenClose )? ;
enumImpl            : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
enumSpec            : enumInit | enumVal | enumDef | enumCase | enumImpl ;
defenumStatement    : DefEnumKeyword symbolIdentifier CurlyOpen enumSpec* CurlyClose ;


// defstruct statement

structSuper         : FromKeyword assignableType ParenOpen argList? ParenClose ;
structInit          : InitKeyword paramSpec structSuper? CurlyOpen block? CurlyClose ;
structVal           : ValKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator defaultInitializer )? ;
structDef           : DefKeyword symbolIdentifier paramSpec returnSpec CurlyOpen block CurlyClose ;
structImpl          : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
structDerives       : ( SealedKeyword | FinalKeyword ) ;
structSpec          : structInit | structVal | structDef | structImpl ;
defstructStatement  : DefStructKeyword symbolIdentifier structDerives? CurlyOpen structSpec* CurlyClose ;


// global statement

globalStatement     : GlobalKeyword ( ValKeyword | VarKeyword )
                        symbolIdentifier ColonOperator assignableType AssignOperator defaultInitializer ;


// defalias statement

defaliasStatement   : DefAliasKeyword Identifier placeholderSpec? assignableType ;


// namespace statement

namespaceStatement  : NamespaceKeyword symbolIdentifier CurlyOpen block CurlyClose ;


// symbol management statements

moduleLocation      : StringLiteral ;

importRef           : symbolPath symbolAlias? ;
importSet           : CurlyOpen importRef ( CommaOperator importRef )* CurlyClose ;
importSpec          : importRef | importSet ;
importStatement     : ImportKeyword FromKeyword moduleLocation EllipsisOperator         # importAllStatement
                    | ImportKeyword FromKeyword moduleLocation importSpec               # importSymbolsStatement
                    | ImportKeyword moduleLocation NamedKeyword Identifier              # importModuleStatement
                    ;

exportRef           : symbolPath symbolAlias? ;
exportSet           : CurlyOpen exportRef ( CommaOperator exportRef )* CurlyClose ;
exportSpec          : exportRef | exportSet ;
exportStatement     : ExportKeyword FromKeyword moduleLocation EllipsisOperator         # exportAllStatement
                    | ExportKeyword FromKeyword moduleLocation exportSpec               # exportSymbolsStatement
                    | ExportKeyword moduleLocation NamedKeyword Identifier              # exportModuleStatement
                    ;

usingPath           : Identifier ( DotOperator Identifier )? ;
usingSet            : CurlyOpen usingPath ( CommaOperator usingPath )* CurlyClose ;
usingSpec           : usingPath | usingSet ;
usingStatement      : UsingKeyword FromKeyword moduleLocation usingSpec                 # usingFromStatement
                    | UsingKeyword usingSpec                                            # usingLocalStatement
                    ;


// lambda expression

lambdaExpression    : LambdaKeyword paramSpec returnSpec CurlyOpen block CurlyClose ;


// return statement

returnStatement     : ReturnKeyword expression ;


// if statement

ifStatement         : IfKeyword basicExpression CurlyOpen block CurlyClose ;


// if-then-else expression

ifThenElseExpression: IfKeyword basicExpression ThenKeyword basicExpression ElseKeyword basicExpression ;


// cond expression

condWhen            : WhenKeyword expression block ;
condElse            : ElseKeyword block ;
condExpression      : CondKeyword CurlyOpen condWhen+ condElse CurlyClose ;


// cond-if statement

condIfWhen          : WhenKeyword expression block ;
condIfElse          : ElseKeyword block ;
condIfStatement     : CondKeyword IfKeyword CurlyOpen condIfWhen+ condIfElse? CurlyClose ;


// match expression

matchTarget         : expression ;
unwrapParam         : VarKeyword? Identifier paramType ;
unwrapList          : unwrapParam ( CommaOperator unwrapParam )* ;
unwrapSpec          : assignableType ParenOpen unwrapList? ParenClose ;

matchWhen           : WhenKeyword ( Identifier ColonOperator )? unwrapSpec block        # matchOnUnwrap
                    | WhenKeyword Identifier ColonOperator assignableType block         # matchOnType
                    | WhenKeyword symbolPath block                                      # matchOnEnum
                    ;
matchElse           : ElseKeyword block ;
matchExpression     : MatchKeyword matchTarget CurlyOpen matchWhen+ matchElse? CurlyClose ;


// while statement

whileStatement      : WhileKeyword expression CurlyOpen block CurlyClose ;


// for statement

forStatement        : ForKeyword Identifier ( ColonOperator assignableType )?
                        InKeyword basicExpression
                        CurlyOpen block CurlyClose ;

// try statement

tryTarget           : block ;
catchWhen           : WhenKeyword ( Identifier ColonOperator )? unwrapSpec block        # catchOnUnwrap
                    | WhenKeyword Identifier ColonOperator assignableType block         # catchOnType
                    ;
catchElse           : ElseKeyword block ;
catchFinally        : FinallyKeyword block ;
tryStatement        : TryKeyword CurlyOpen tryTarget catchWhen* catchElse? catchFinally? CurlyClose ;

// basic expressions

basicExpression     : basicExpression ArrowOperator basicExpression                         # pairExpression
                    | basicExpression AndKeyword basicExpression                            # booleanAndExpression
                    | basicExpression OrKeyword basicExpression                             # booleanOrExpression
                    | NotKeyword basicExpression                                            # booleanNotExpression
                    | equality                                                              # equalityRule
                    ;

equality            : equality IsAOperator assignableType                                   # isAExpression
                    | equality IsEqOperator equality                                        # isEqualExpression
                    | equality IsLtOperator equality                                        # isLessThanExpression
                    | equality IsLeOperator equality                                        # isLessOrEqualExpression
                    | equality IsGtOperator equality                                        # isGreaterThanExpression
                    | equality IsGeOperator equality                                        # isGreaterOrEqualExpression
                    | plusOrMinus                                                           # plusOrMinusRule
                    ;

plusOrMinus         : plusOrMinus PlusOperator multOrDiv                                    # addExpression
                    | plusOrMinus MinusOperator multOrDiv                                   # subExpression
                    | multOrDiv                                                             # mulOrDivRule
                    ;

multOrDiv           : multOrDiv StarOperator negation                                       # mulExpression
                    | multOrDiv SlashOperator negation                                      # divExpression
                    | negation                                                              # negationRule
                    ;

negation            : MinusOperator negation                                                # negExpression
                    | newOrDeref                                                            # newOrDerefRule
                    ;

newOrDeref          : derefLiteral derefSpec*                                               # literalExpression
                    | derefGrouping derefSpec*                                              # groupingExpression
                    | derefNew derefSpec*                                                   # newExpression
                    | thisSpec derefSpec*                                                   # thisExpression
                    | callSpec derefSpec*                                                   # callExpression
                    | nameSpec derefSpec*                                                   # nameExpression
                    ;

derefLiteral        : literal ;
derefGrouping       : ParenOpen expression ParenClose ;
derefNew            : assignableType CurlyOpen argList? CurlyClose ;
thisSpec            : ThisKeyword ;
callSpec            : Identifier typeArguments? ParenOpen argList? ParenClose ;
nameSpec            : Identifier ;

derefSpec           : DotOperator
                        Identifier typeArguments? ParenOpen argList? ParenClose             # derefMethod
                    | DotOperator Identifier                                                # derefMember
                    ;


// literal structures

literal             : numberLiteral | textLiteral | symbolLiteral | keywordLiteral ;

decimalInteger      : DecimalInteger ;
hexInteger          : HexInteger ;
octalInteger        : OctalInteger ;
decimalFixedFloat   : DecimalFixedFloat ;
decimalScientificFloat:  DecimalScientificFloat ;
hexFloat            : HexFloat ;
numberLiteral       : decimalInteger
                    | hexInteger
                    | octalInteger
                    | decimalFixedFloat
                    | decimalScientificFloat
                    | hexFloat
                    ;

charLiteral         : CharLiteral ;
stringLiteral       : StringLiteral ;
urlLiteral          : UrlLiteral ;
textLiteral         : charLiteral | stringLiteral | urlLiteral ;

symbolLiteral       : HashOperator Identifier ( DotOperator Identifier)* ;

trueLiteral         : TrueKeyword ;
falseLiteral        : FalseKeyword ;
undefLiteral        : UndefKeyword ;
nilLiteral          : NilKeyword ;
keywordLiteral      : trueLiteral | falseLiteral | undefLiteral | nilLiteral ;


// macros and annotations

macroCall           : Identifier ( ParenOpen argList? ParenClose )? ;
macro               : AtOperator CurlyOpen macroCall+ CurlyClose ;

annotation          : AtOperator macroCall ;
annotationList      : annotation+ ;


// comment

comment             : SingleLineComment | MultiLineComment ;

