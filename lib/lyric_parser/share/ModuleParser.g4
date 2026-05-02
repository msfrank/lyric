parser grammar ModuleParser;

options { tokenVocab = ModuleLexer; }


root                : pragmaMacro* block EOF ;
block               : form+ ;


// a form is an individually parseable unit of code

form                : blockMacro
                    | expression
                    | statement
                    ;


// forms which return a value

expression          : <assoc=right> expression ThenKeyword expression
                        ElseKeyword expression                                      # ternaryExpression
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


// forms which do not return a value

statement           : controlStatement
                    | definitionStatement
                    | typenameStatement
                    | importStatement
                    | usingStatement
                    | setStatement
                    ;

definitionStatement : valStatement
                    | varStatement
                    | defStatement
                    | defaliasStatement
                    | defclassStatement
                    | defconceptStatement
                    | defenumStatement
                    | definstanceStatement
                    | defstaticStatement
                    | defstructStatement
                    | namespaceStatement
                    ;

controlStatement    : ifStatement
                    | doStatement
                    | whileStatement
                    | forStatement
                    | tryStatement
                    | returnStatement
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

typeParameterList   : assignableType ( CommaOperator assignableType )*
                    | assignableType ( CommaOperator assignableType )* CommaOperator ( CommaOperator assignableType )*
                        { notifyErrorListeners("Extra ',' after type parameter in the parametric type"); }
                    | assignableType ( CommaOperator assignableType )* assignableType ( CommaOperator assignableType )*
                        { notifyErrorListeners("Missing ',' between type parameters in the parametric type"); }
                    | CommaOperator assignableType ( CommaOperator assignableType )*
                        { notifyErrorListeners("Extra ',' before type parameter in the parametric type"); }
                    ;

parametricType      : simpleType BracketOpen typeParameterList BracketClose
                    | simpleType BracketOpen BracketOpen typeParameterList BracketClose
                        { notifyErrorListeners("Extra '[' opening parametric type"); }
                    | simpleType BracketOpen typeParameterList BracketClose BracketClose
                        { notifyErrorListeners("Extra ']' closing parametric type"); }
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
                    | statement
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

// initializers

initializerNamedNew             : symbolPath typeArguments? CurlyOpen argumentList? CurlyClose
                                | symbolPath typeArguments? CurlyOpen argumentList? CurlyClose CurlyClose
                                    { notifyErrorListeners("Extra '}' closing default initializer"); }
                                | symbolPath typeArguments? CurlyOpen argumentList?
                                    { notifyErrorListeners("Missing '}' closing default initializer"); }
                                | symbolPath typeArguments?
                                    { notifyErrorListeners("Missing default initializer argument list"); }
                                ;
initializerDefaultNew           : CurlyOpen argumentList? CurlyClose
                                | CurlyOpen argumentList? CurlyClose CurlyClose
                                    { notifyErrorListeners("Extra '}' closing default initializer"); }
                                | CurlyOpen argumentList?
                                    { notifyErrorListeners("Missing '}' closing default initializer"); }
                                ;
initializer                     : literal
                                | initializerDefaultNew
                                | initializerNamedNew
                                ;


// parameter lists

paramType           : ColonOperator assignableType ;
paramDefault        : AssignOperator initializer ;
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


// variable declaration

symbolAndType       : symbolIdentifier ColonOperator assignableType ;
variableDefaultNew  : CurlyOpen argumentList? CurlyClose
                    | CurlyOpen argumentList? CurlyClose CurlyClose
                        { notifyErrorListeners("Extra '}' closing variable initializer"); }
                    | CurlyOpen argumentList?
                        { notifyErrorListeners("Missing '}' closing variable initializer"); }
                    ;

valStatement        : ValKeyword symbolAndType AssignOperator variableDefaultNew        # defaultNewVal
                    | ValKeyword symbolAndType AssignOperator expression                # typedVal
                    | ValKeyword symbolIdentifier AssignOperator expression             # untypedVal
                    ;

varStatement        : VarKeyword symbolAndType AssignOperator variableDefaultNew        # defaultNewVar
                    | VarKeyword symbolAndType AssignOperator expression                # typedVar
                    | VarKeyword symbolIdentifier AssignOperator expression             # untypedVar
                    ;


// variable assignment

assignmentSpec      : ( ThisKeyword | Identifier ) ( DotOperator Identifier )+          # memberAssignment
                    | Identifier                                                        # nameAssignment
                    ;

assignOp            : AssignOperator
                    | PlusAssignOperator
                    | MinusAssignOperator
                    | StarAssignOperator
                    | SlashAssignOperator
                    ;

assignDefaultNew    : CurlyOpen argumentList? CurlyClose
                    | CurlyOpen argumentList? CurlyClose CurlyClose
                        { notifyErrorListeners("Extra '}' closing assignment initializer"); }
                    | CurlyOpen argumentList?
                        { notifyErrorListeners("Missing '}' closing assignment initializer"); }
                    ;

setStatement        : assignmentSpec assignOp assignDefaultNew                          # defaultNewSet
                    | assignmentSpec assignOp expression                                # expressionSet
                    ;


//

initBase            : FromKeyword ThisKeyword callArguments                             # defaultThisBase
                    | FromKeyword ThisKeyword DotOperator Identifier callArguments      # namedThisBase
                    | FromKeyword SuperKeyword callArguments                            # defaultSuperBase
                    | FromKeyword SuperKeyword DotOperator Identifier callArguments     # namedSuperBase
                    ;


// proc block

procBlock           : ArrowOperator basicExpression
                    | ArrowOperator
                        { notifyErrorListeners("Expected basic expression after '->'"); }
                    | CurlyOpen block? CurlyClose
                    | CurlyOpen CurlyOpen block? CurlyClose
                        { notifyErrorListeners("Extra '{' opening def"); }
                    | CurlyOpen block? CurlyClose CurlyClose
                        { notifyErrorListeners("Extra '}' closing def"); }
                    | CurlyOpen block?
                        { notifyErrorListeners("Missing '}' closing def"); }
                    ;


// def statement

defStatement        : definitionMacro? DefKeyword
                        symbolIdentifier placeholderSpec? paramSpec returnSpec? constraintSpec?
                        procBlock ;

// impl spec

implDef             : DefKeyword symbolIdentifier paramSpec returnSpec? procBlock ;
implSpec            : implDef ;


// global spec

globalVal           : definitionMacro? ValKeyword
                        symbolIdentifier ColonOperator assignableType AssignOperator initializer ;
globalVar           : definitionMacro? VarKeyword
                        symbolIdentifier ColonOperator assignableType AssignOperator initializer ;
globalSpec          : globalVal | globalVar | defStatement ;


// modifiers spec

sealedModifier      : SealedKeyword ;
finalModifier       : FinalKeyword ;
abstractModifier    : AbstractKeyword ;
modifierSpec       : sealedModifier | finalModifier | abstractModifier ;


// defclass statement

defclassStatement   : definitionMacro? DefClassKeyword symbolIdentifier
                        genericClass? modifierSpec* classBase?
                        CurlyOpen classSpec* CurlyClose ;
genericClass        : placeholderSpec constraintSpec? ;
classBase           : FromKeyword assignableType ;
classInit           : InitKeyword symbolIdentifier? paramSpec initBase? procBlock ;
classVal            : ValKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator initializer )? ;
classVar            : VarKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator initializer )? ;
classDef            : DefKeyword symbolIdentifier
                        placeholderSpec? paramSpec returnSpec? constraintSpec? FinalKeyword?
                        procBlock ;
classDecl           : DeclKeyword symbolIdentifier paramSpec returnSpec? ;
classImpl           : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
classGlobal         : GlobalKeyword CurlyOpen globalSpec* CurlyClose ;
classSpec           : classInit | classVal | classVar | classDef | classDecl | classImpl | classGlobal ;


// defconcept statement

defconceptStatement : definitionMacro? DefConceptKeyword symbolIdentifier
                        genericConcept? modifierSpec* conceptBase?
                        CurlyOpen conceptSpec* CurlyClose ;
genericConcept      : placeholderSpec constraintSpec? ;
conceptBase         : FromKeyword assignableType ;
conceptDecl         : DeclKeyword symbolIdentifier paramSpec returnSpec? ;
conceptImpl         : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
conceptGlobal       : GlobalKeyword CurlyOpen globalSpec* CurlyClose ;
conceptSpec         : conceptDecl | conceptImpl | conceptGlobal ;


// definstance statement

definstanceStatement: definitionMacro? DefInstanceKeyword symbolIdentifier
                        modifierSpec* instanceBase?
                        CurlyOpen instanceSpec* CurlyClose ;
instanceBase        : FromKeyword assignableType ;
instanceInit        : InitKeyword paramSpec procBlock ;
instanceVal         : ValKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator initializer )? ;
instanceVar         : VarKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator initializer )? ;
instanceDef         : DefKeyword symbolIdentifier paramSpec returnSpec? procBlock ;
instanceImpl        : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
instanceGlobal      : GlobalKeyword CurlyOpen globalSpec* CurlyClose ;
instanceSpec        : instanceInit | instanceVal | instanceVar | instanceDef | instanceImpl | instanceGlobal ;


// defenum statement

defenumStatement    : definitionMacro? DefEnumKeyword symbolIdentifier
                        modifierSpec* enumBase?
                        CurlyOpen enumSpec* CurlyClose ;
enumBase            : FromKeyword assignableType ;
enumInit            : InitKeyword paramSpec procBlock ;
enumVal             : ValKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator initializer )? ;
enumDef             : DefKeyword symbolIdentifier paramSpec returnSpec? procBlock ;
enumCase            : CaseKeyword symbolIdentifier callArguments? ;
enumImpl            : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
enumGlobal          : GlobalKeyword CurlyOpen globalSpec* CurlyClose ;
enumSpec            : enumInit | enumVal | enumDef | enumCase | enumImpl | enumGlobal ;


// defstruct statement

defstructStatement  : definitionMacro? DefStructKeyword symbolIdentifier
                        modifierSpec* structBase?
                        CurlyOpen structSpec* CurlyClose ;
structBase          : FromKeyword assignableType ;
structInit          : InitKeyword symbolIdentifier? paramSpec initBase? procBlock ;
structVal           : ValKeyword symbolIdentifier ColonOperator assignableType ( AssignOperator initializer )? ;
structDef           : DefKeyword symbolIdentifier paramSpec returnSpec? procBlock ;
structImpl          : ImplKeyword assignableType CurlyOpen implSpec* CurlyClose ;
structGlobal        : GlobalKeyword CurlyOpen globalSpec* CurlyClose ;
structSpec          : structInit | structVal | structDef | structImpl | structGlobal ;


// defstatic statement

defstaticStatement  : definitionMacro? GlobalKeyword ( ValKeyword | VarKeyword )
                        symbolIdentifier ColonOperator assignableType AssignOperator initializer ;


// defalias statement

defaliasStatement   : definitionMacro? DefAliasKeyword symbolIdentifier placeholderSpec? FromKeyword assignableType ;


// namespace statement

namespaceSpec       : defStatement
                    | defclassStatement
                    | defconceptStatement
                    | defenumStatement
                    | definstanceStatement
                    | defstaticStatement
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

// clause block

clauseBlock         : ArrowOperator basicExpression
                    | ArrowOperator
                        { notifyErrorListeners("Expected basic expression after '->'"); }
                    | CurlyOpen block CurlyClose
                    | CurlyOpen CurlyOpen block CurlyClose
                        { notifyErrorListeners("Extra '{' opening clause"); }
                    | CurlyOpen block CurlyClose CurlyOpen
                        { notifyErrorListeners("Extra '}' closing clause"); }
                    | CurlyOpen block?
                        { notifyErrorListeners("Missing '}' closing clause"); }
                    ;

// lambda expression

lambdaExpression    : LambdaKeyword paramSpec returnSpec? procBlock ;


// lambda-from expression

lambdaFromExpression: LambdaKeyword FromKeyword symbolPath ;


// return statement

returnStatement     : ReturnKeyword expression ;


// if statement

ifStatement         : IfKeyword basicExpression clauseBlock ;


// do statement

doWhen              : WhenKeyword expression clauseBlock
                    | WhenKeyword expression
                        { notifyErrorListeners("Missing '{' or '->' for when clause"); }
                    | WhenKeyword
                        { notifyErrorListeners("Missing '{' or '->' for when clause"); }
                    ;
doElse              : ElseKeyword clauseBlock
                    | ElseKeyword
                        { notifyErrorListeners("Missing '{' or '->' for else clause"); }
                    ;
doStatement         : DoKeyword CurlyOpen doWhen+ doElse? CurlyClose ;


// cond expression

condWhen            : WhenKeyword expression clauseBlock
                    | WhenKeyword expression
                        { notifyErrorListeners("Missing '{' or '->' for when clause"); }
                    | WhenKeyword
                        { notifyErrorListeners("Missing '{' or '->' for when clause"); }
                    ;
condElse            : ElseKeyword clauseBlock
                    | ElseKeyword
                        { notifyErrorListeners("Missing '{' or '->' for else clause"); }
                    ;
condExpression      : CondKeyword CurlyOpen condWhen+ condElse CurlyClose ;


// match expression

matchTarget         : expression ;
unwrapParam         : VarKeyword? Identifier paramType ;
unwrapList          : unwrapParam ( CommaOperator unwrapParam )* ;
unwrapSpec          : assignableType ParenOpen unwrapList? ParenClose ;

matchWhen           : WhenKeyword ( Identifier ColonOperator )? unwrapSpec clauseBlock      # matchOnUnwrap
                    | WhenKeyword Identifier ColonOperator assignableType clauseBlock       # matchOnType
                    | WhenKeyword symbolPath clauseBlock                                    # matchOnEnum
                    ;
matchElse           : ElseKeyword clauseBlock ;
matchExpression     : MatchKeyword matchTarget CurlyOpen matchWhen+ matchElse? CurlyClose ;


// while statement

whileStatement      : WhileKeyword expression clauseBlock ;


// for statement

forStatement        : ForKeyword Identifier ( ColonOperator assignableType )?
                        InKeyword basicExpression
                        clauseBlock ;


// try statement

tryBlock            : TryKeyword CurlyOpen block CurlyClose ;
tryCatch            : CatchKeyword CurlyOpen catchWhen* catchElse? CurlyClose ;
tryFinally          : FinallyKeyword CurlyOpen block CurlyClose ;
catchWhen           : WhenKeyword ( Identifier ColonOperator )? unwrapSpec clauseBlock      # catchOnUnwrap
                    | WhenKeyword Identifier ColonOperator assignableType clauseBlock       # catchOnType
                    ;
catchElse           : ElseKeyword block ;
tryStatement        : tryBlock tryCatch tryFinally? ;


// expect expression

expectExpression    : ExpectKeyword expression ;


// raise expression

raiseExpression     : RaiseKeyword expression ;


// basic expressions

basicExpression     : basicExpression AndKeyword basicExpression                            # booleanAndExpression
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

plusOrMinus         : plusOrMinus PlusOperator plusOrMinus                                  # addExpression
                    | plusOrMinus MinusOperator plusOrMinus                                 # subExpression
                    | starOrSlash                                                           # mulOrDivRule
                    ;

starOrSlash         : starOrSlash StarOperator starOrSlash                                  # mulExpression
                    | starOrSlash SlashOperator starOrSlash                                 # divExpression
                    | symbol                                                                # symbolRule
                    ;

symbol              : HashOperator Identifier ( DotOperator Identifier)*                    # symbolExpression
                    | typeofOrCast                                                          # typeofOrCastRule
                    ;

typeofOrCast        : TypeOfKeyword assignableType                                          # typeofExpression
                    | typeofOrCast AsKeyword assignableType                                 # castExpression
                    | literalOrNeg                                                          # literalOrNegRule
                    ;

literalOrNeg        : derefLiteral derefSpec*                                               # literalExpression
                    | MinusOperator literalOrNeg                                            # negExpression
                    | newOrDeref                                                            # newOrDerefRule
                    ;

newOrDeref          : derefGrouping derefSpec*                                              # groupingExpression
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

decimalInteger          : MinusOperator? DecimalInteger ;
decimalFixedFloat       : MinusOperator? DecimalFixedFloat ;
decimalScientificFloat  : MinusOperator? CoefficientLeadingPeriod DecimalExponent
                        | MinusOperator? CoefficientWithPeriod DecimalExponent
                        | MinusOperator? CoefficientNoPeriod DecimalExponent
                        ;
hexInteger              : MinusOperator? HexPrefix HexInteger ;
hexFloat                : MinusOperator? HexPrefix ( HexFloatTrailingPeriod | HexFloatLeadingPeriod | HexFloatNoPeriod );
octalInteger            : MinusOperator? OctPrefix OctalInteger ;

invalidNumber           : CoefficientLeadingPeriod InvalidExponent
                        | CoefficientWithPeriod InvalidExponent
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
rawLiteral              : RawLiteral ;
textLiteral             : charLiteral | stringLiteral | rawLiteral ;


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

