parser grammar ModuleParser;

options { tokenVocab = ModuleLexer; }


root                : pragmaMacro* block EOF ;
block               : form+ ;


// a form is an individually parseable unit of code

form                : blockMacro
                    | expression
                    | typenameStatement
                    | valStatement
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
                    | doStatement
                    | whileStatement
                    | forStatement
                    | tryStatement
                    | returnStatement
                    | importStatement
                    | usingStatement
                    | basicExpression
                    ;


// forms which return a value

expression          : <assoc=right> expression
                        ThenKeyword expression ElseKeyword expression               # ternaryExpression
                    | namedExpression                                               # namedLabel
                    | basicExpression                                               # basicLabel
                    ;

namedExpression     : condExpression
                    | matchExpression
                    | lambdaFromExpression
                    | lambdaExpression
                    | expectExpression
                    | raiseExpression
                    ;

// symbols

symbolIdentifier    : Identifier ;
symbolPath          : Identifier ( DotOperator Identifier )*
                    | Identifier ( DotOperator Identifier )* DotOperator DotOperator
                        { notifyErrorListeners("Extra '.' between identifiers in the symbol path"); }
                    | Identifier ( DotOperator Identifier )* DotOperator DotOperator
                        { notifyErrorListeners("Extra '.' at the end of the symbol path"); }
                    ;

// types

simpleType          : Identifier ( DotOperator Identifier )*
                    | Identifier ( DotOperator Identifier )* DotOperator
                        { notifyErrorListeners("Extra '.' after identifier in the type path"); }
                    ;
parametricType      : simpleType BracketOpen assignableType ( CommaOperator assignableType )* BracketClose
                    | simpleType BracketOpen assignableType ( CommaOperator assignableType )* BracketClose BracketClose
                        { notifyErrorListeners("Extra ']' closing parametric type"); }
                    | simpleType BracketOpen assignableType ( CommaOperator assignableType )* CommaOperator
                        { notifyErrorListeners("Extra ',' after type parameter in the parametric type"); }
                    | simpleType BracketOpen assignableType ( CommaOperator assignableType )* assignableType
                        { notifyErrorListeners("Missing ',' between type parameters in the parametric type"); }
                    | simpleType BracketOpen CommaOperator
                        { notifyErrorListeners("Extra ',' before type parameter in the parametric type"); }
                    | simpleType BracketOpen BracketClose
                        { notifyErrorListeners("Parametric type must contain at least one type parameter"); }
                    ;
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

argument            : Identifier AssignOperator expression
                    | expression
                    | form
                        { notifyErrorListeners("argument must be a valid expression or keyword argument"); }
                    ;
argumentList        : argument ( CommaOperator argument )*
                    | argument ( CommaOperator argument )* CommaOperator
                        { notifyErrorListeners("Extra ',' after argument in the argument list"); }
                    | argument ( CommaOperator argument )* argument
                        { notifyErrorListeners("Missing ',' between arguments in the argument list"); }
                    | CommaOperator
                        { notifyErrorListeners("Extra ',' before argument in the argument list"); }
                    ;
callArguments       : ParenOpen argumentList? ParenClose
                    ;
newArguments        : CurlyOpen argumentList? CurlyClose
                    ;

// initializer

defaultInitializerTypedNew      : assignableType CurlyOpen argumentList? CurlyClose
                                | assignableType CurlyOpen argumentList? CurlyClose CurlyClose
                                    { notifyErrorListeners("Extra '}' closing default initializer"); }
                                | assignableType CurlyOpen argumentList?
                                    { notifyErrorListeners("Missing '}' closing default initializer"); }
                                | assignableType
                                    { notifyErrorListeners("Missing default initializer argument list"); }
                                ;
defaultInitializerNew           : CurlyOpen argumentList? CurlyClose
                                | CurlyOpen argumentList? CurlyClose CurlyClose
                                    { notifyErrorListeners("Extra '}' closing default initializer"); }
                                | CurlyOpen argumentList?
                                    { notifyErrorListeners("Missing '}' closing default initializer"); }
                                ;
defaultInitializerLiteral       : literal ;
defaultInitializer              : defaultInitializerTypedNew
                                | defaultInitializerNew
                                | defaultInitializerLiteral
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

rest                : Identifier ColonOperator EllipsisOperator assignableType
                    | EllipsisOperator assignableType
                    | EllipsisOperator
                        { notifyErrorListeners("Missing type for rest parameter"); }
                    ;

paramList           : rest
                    | param ( CommaOperator param )* ( CommaOperator rest )?
                    | param ( CommaOperator param )* ( CommaOperator rest )? CommaOperator
                        { notifyErrorListeners("Extra ',' after parameter in the parameter list"); }
                    | rest CommaOperator
                        { notifyErrorListeners("Rest parameter must be the last parameter in the parameter list"); }
                    | CommaOperator
                        { notifyErrorListeners("Extra ',' before parameter in the parameter list"); }
                    ;
paramSpec           : ParenOpen paramList? ParenClose
                    | ParenOpen paramList? ParenClose ParenClose
                        { notifyErrorListeners("Extra ')' closing parameter list"); }
                    | ParenOpen paramList?
                        { notifyErrorListeners("Missing ')' closing parameter list"); }
                    ;
returnSpec          : ColonOperator assignableType
                    | ColonOperator
                        { notifyErrorListeners("Missing return type after ':'"); }
                    | assignableType
                        { notifyErrorListeners("Missing ':' before return type"); }
                    ;


// typename statement

typenameStatement   : TypeNameKeyword symbolIdentifier ;


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


//

initBase            : FromKeyword ThisKeyword callArguments                             # defaultThisBase
                    | FromKeyword ThisKeyword DotOperator Identifier callArguments      # namedThisBase
                    | FromKeyword SuperKeyword callArguments                            # defaultSuperBase
                    | FromKeyword SuperKeyword DotOperator Identifier callArguments     # namedSuperBase
                    ;


// proc block

procBlock           : CurlyOpen block? CurlyClose
                    | CurlyOpen block? CurlyClose CurlyClose
                        { notifyErrorListeners("Extra '}' closing def"); }
                    | CurlyOpen block
                        { notifyErrorListeners("Missing '}' closing def"); }
                    ;


// def statement

defStatement        : definitionMacro? DefKeyword
                        symbolIdentifier placeholderSpec? paramSpec returnSpec? constraintSpec?
                        procBlock ;

// impl statement

implDef             : DefKeyword symbolIdentifier paramSpec returnSpec? procBlock ;
implSpec            : implDef ;


// defclass statement

defclassStatement   : definitionMacro? DefClassKeyword symbolIdentifier
                        genericClass? classDerives? classBase?
                        CurlyOpen classSpec* CurlyClose ;
genericClass        : placeholderSpec constraintSpec? ;
classDerives        : ( SealedKeyword | FinalKeyword ) ;
classBase           : FromKeyword assignableType ;
classInit           : InitKeyword symbolIdentifier? paramSpec initBase? procBlock ;
classVal            : ValKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator defaultInitializer )? ;
classVar            : VarKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator defaultInitializer )? ;
classDef            : DefKeyword symbolIdentifier
                        placeholderSpec? paramSpec returnSpec? constraintSpec? FinalKeyword?
                        procBlock ;
classDecl           : DeclKeyword symbolIdentifier paramSpec returnSpec? ;
classImpl           : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
classSpec           : classInit | classVal | classVar | classDef | classDecl | classImpl ;


// defconcept statement

defconceptStatement : definitionMacro? DefConceptKeyword symbolIdentifier
                        genericConcept? conceptDerives? conceptBase?
                        CurlyOpen conceptSpec* CurlyClose ;
genericConcept      : placeholderSpec constraintSpec? ;
conceptDerives      : ( SealedKeyword | FinalKeyword ) ;
conceptBase         : FromKeyword assignableType ;
conceptDecl         : DeclKeyword symbolIdentifier paramSpec returnSpec? ;
conceptImpl         : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
conceptSpec         : conceptDecl | conceptImpl ;


// definstance statement

definstanceStatement: definitionMacro? DefInstanceKeyword symbolIdentifier
                        instanceDerives? instanceBase?
                        CurlyOpen instanceSpec* CurlyClose ;
instanceDerives     : ( SealedKeyword | FinalKeyword ) ;
instanceBase        : FromKeyword assignableType ;
instanceInit        : InitKeyword paramSpec procBlock ;
instanceVal         : ValKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator defaultInitializer )? ;
instanceVar         : VarKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator defaultInitializer )? ;
instanceDef         : DefKeyword symbolIdentifier paramSpec returnSpec? procBlock ;
instanceImpl        : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
instanceSpec        : instanceInit | instanceVal | instanceVar | instanceDef | instanceImpl ;


// defenum statement

defenumStatement    : definitionMacro? DefEnumKeyword symbolIdentifier
                        enumBase?
                        CurlyOpen enumSpec* CurlyClose ;
enumBase            : FromKeyword assignableType ;
enumInit            : InitKeyword paramSpec procBlock ;
enumVal             : ValKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator defaultInitializer )? ;
enumDef             : DefKeyword symbolIdentifier paramSpec returnSpec? procBlock ;
enumCase            : CaseKeyword symbolIdentifier callArguments? ;
enumImpl            : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
enumSpec            : enumInit | enumVal | enumDef | enumCase | enumImpl ;


// defstruct statement

defstructStatement  : definitionMacro? DefStructKeyword symbolIdentifier
                        structDerives? structBase?
                        CurlyOpen structSpec* CurlyClose ;
structDerives       : ( SealedKeyword | FinalKeyword ) ;
structBase          : FromKeyword assignableType ;
structInit          : InitKeyword symbolIdentifier? paramSpec initBase? procBlock ;
structVal           : ValKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator defaultInitializer )? ;
structDef           : DefKeyword symbolIdentifier paramSpec returnSpec? procBlock ;
structImpl          : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
structSpec          : structInit | structVal | structDef | structImpl ;


// global statement

globalStatement     : definitionMacro? GlobalKeyword ( ValKeyword | VarKeyword )
                        symbolIdentifier ColonOperator assignableType AssignOperator defaultInitializer ;


// defalias statement

defaliasStatement   : definitionMacro? DefAliasKeyword symbolIdentifier placeholderSpec? FromKeyword assignableType ;


// namespace statement

namespaceSpec       : globalStatement
                    | defStatement
                    | defclassStatement
                    | defconceptStatement
                    | defenumStatement
                    | definstanceStatement
                    | defstructStatement
                    ;
namespaceStatement  : definitionMacro? NamespaceKeyword
                        symbolIdentifier
                        CurlyOpen namespaceSpec* CurlyClose ;


// symbol management statements

moduleLocation      : StringLiteral ;
symbolAlias         : NamedKeyword Identifier ;

importRef           : symbolPath symbolAlias? ;
importSet           : CurlyOpen importRef ( CommaOperator importRef )* CurlyClose ;
importSpec          : importRef | importSet ;
importStatement     : ImportKeyword FromKeyword moduleLocation EllipsisOperator         # importAllStatement
                    | ImportKeyword FromKeyword moduleLocation importSpec               # importSymbolsStatement
                    | ImportKeyword moduleLocation NamedKeyword Identifier              # importModuleStatement
                    ;

usingRef            : newOrDeref ;
usingType           : singularType ;
usingSet            : CurlyOpen usingType ( CommaOperator usingType )* CurlyClose ;
usingAll            : UsingKeyword usingRef ;
usingImpls          : UsingKeyword FromKeyword usingRef usingSet ;
usingStatement      : usingAll | usingImpls ;


// lambda expression

lambdaExpression    : LambdaKeyword paramSpec returnSpec? procBlock ;


// lambda-from expression

lambdaFromExpression: LambdaKeyword FromKeyword symbolPath ;


// return statement

returnStatement     : ReturnKeyword expression ;


// if statement

ifStatement         : IfKeyword basicExpression CurlyOpen block CurlyClose ;


// do statement

doWhen              : WhenKeyword expression block ;
doElse              : ElseKeyword block ;
doStatement         : DoKeyword CurlyOpen doWhen+ doElse? CurlyClose ;


// cond expression

condWhen            : WhenKeyword expression block ;
condElse            : ElseKeyword block ;
condExpression      : CondKeyword CurlyOpen condWhen+ condElse CurlyClose ;


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

tryBlock            : TryKeyword CurlyOpen block CurlyClose ;
tryCatch            : CatchKeyword CurlyOpen catchWhen* catchElse? CurlyClose ;
tryFinally          : FinallyKeyword CurlyOpen block CurlyClose ;
catchWhen           : WhenKeyword ( Identifier ColonOperator )? unwrapSpec block        # catchOnUnwrap
                    | WhenKeyword Identifier ColonOperator assignableType block         # catchOnType
                    ;
catchElse           : ElseKeyword block ;
tryStatement        : tryBlock tryCatch tryFinally? ;


// expect expression

expectExpression    : ExpectKeyword expression ;


// raise expression

raiseExpression     : RaiseKeyword expression ;


// basic expressions

basicExpression     : basicExpression ArrowOperator basicExpression                         # pairExpression
                    | basicExpression AndKeyword basicExpression                            # booleanAndExpression
                    | basicExpression OrKeyword basicExpression                             # booleanOrExpression
                    | NotKeyword basicExpression                                            # booleanNotExpression
                    | equality                                                              # equalityRule
                    ;

equality            : equality IsEqOperator equality                                        # isEqualExpression
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
                    | symbol                                                                # symbolRule
                    ;

symbol              : HashOperator Identifier ( DotOperator Identifier)*                    # symbolExpression
                    | typeof                                                                # typeofRule
                    ;

typeof              : TypeOfKeyword assignableType                                          # typeofExpression
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
derefNew            : symbolPath typeArguments? newArguments ;
thisSpec            : ThisKeyword ;
callSpec            : Identifier typeArguments? callArguments ;
nameSpec            : Identifier ;

derefSpec           : DotOperator Identifier typeArguments? callArguments                   # derefMethod
                    | DotOperator Identifier                                                # derefMember
                    ;


// literal structures

literal             : numberLiteral | textLiteral | keywordLiteral ;

decimalInteger          : DecimalInteger ;
decimalFixedFloat       : DecimalFixedFloat ;
decimalScientificFloat  : CoefficientLeadingPeriod DecimalExponent
                        | CoefficientTrailingPeriod DecimalExponent
                        | CoefficientNoPeriod DecimalExponent
                        ;
hexInteger              : HexPrefix HexInteger ;
hexFloat                : HexPrefix ( HexFloatTrailingPeriod | HexFloatLeadingPeriod | HexFloatNoPeriod );
octalInteger            : OctPrefix OctalInteger ;
invalidNumber           : CoefficientLeadingPeriod InvalidExponent
                        | CoefficientTrailingPeriod InvalidExponent
                        | CoefficientNoPeriod InvalidExponent
                        | OctPrefix InvalidOctalLiteral
                        | HexPrefix InvalidHexLiteral
                        ;

numberLiteral           : decimalInteger
                        | decimalFixedFloat
                        | decimalScientificFloat
                        | hexInteger
                        | hexFloat
                        | octalInteger
                        | invalidNumber
                        ;

charLiteral             : CharLiteral ;
stringLiteral           : StringLiteral ;
urlLiteral              : UrlLiteral ;
textLiteral             : charLiteral | stringLiteral | urlLiteral ;


trueLiteral             : TrueKeyword ;
falseLiteral            : FalseKeyword ;
undefLiteral            : UndefKeyword ;
nilLiteral              : NilKeyword ;
keywordLiteral          : trueLiteral | falseLiteral | undefLiteral | nilLiteral ;


// macro forms

macroArgs           : ParenOpen argumentList? ParenClose ;
macroCall           : Identifier macroArgs ;
macroAnnotation     : AtOperator Identifier macroArgs ;

pragmaMacro         : DoubleAtOperator Identifier macroArgs ;
blockMacro          : AtOperator CurlyOpen macroCall+ CurlyClose ;
definitionMacro     : macroAnnotation+ ;


// comment

comment             : SingleLineComment | MultiLineComment ;

