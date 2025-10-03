lexer grammar ModuleLexer;

channels { COMMENTS }

SingleLineComment           : '//' .*? '\n' -> channel(COMMENTS) ;
MultiLineComment            : '/*' .*? '*/' -> channel(COMMENTS) ;

// reserved marker pairs

ParenOpen                   : '(' ;
ParenClose                  : ')' ;
BracketOpen                 : '[' ;
BracketClose                : ']' ;
CurlyOpen                   : '{' ;
CurlyClose                  : '}' ;


// binary literal

BinPrefix                   : '0b' -> pushMode(BINLITERAL) ;


// hexadecimal literal

HexPrefix                   : '0x' -> pushMode(HEXLITERAL) ;


// octal literal

OctPrefix                   : '0o' -> pushMode(OCTLITERAL);


// decimal literal

fragment
DecimalDigit                : ('0'..'9') ;

fragment
DecimalWholeNumber          : ('0'|'1'..'9' '0'..'9'*) ;

fragment
ENotation                   : 'e'|'E' ;

CoefficientLeadingPeriod    : '.' DecimalDigit+ ENotation -> pushMode(EXPONENT) ;
CoefficientTrailingPeriod   : DecimalWholeNumber '.' DecimalDigit+ ENotation -> pushMode(EXPONENT) ;
CoefficientNoPeriod         : DecimalWholeNumber ENotation -> pushMode(EXPONENT) ;
DecimalFixedFloat           : DecimalWholeNumber '.' DecimalDigit* | '.' DecimalDigit+ ;
DecimalInteger              : DecimalWholeNumber ;


// a char literal is a single character or escape sequence enclosed in single-quotes

CharLiteral                 : '\'' ( EscapeSequence | ~('\\'|'"') ) '\'' ;


// a string literal is any sequence of characters (including escape sequences) enclosed in double-quotes

StringLiteral               : '"' ( EscapeSequence | ~('\\'|'"') )* '"' ;

// a url literal is any sequence of characters (including escape sequences) enclosed in backticks

UrlLiteral                  : '`' ( EscapeSequence | ~('\\'|'`') )* '`' ;


fragment
EscapeSequence              : '\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')
                            | Hex1ByteEscape
                            | HexVariableEscape
                            | Unicode2ByteEscape
                            | Unicode4ByteEscape
                            | OctalEscape
                            ;

fragment
OctalEscape                 : '\\' ('0'..'3') ('0'..'7') ('0'..'7')
                            | '\\' ('0'..'7') ('0'..'7')
                            | '\\' ('0'..'7')
                            ;

fragment
Hex1ByteEscape              : '\\' 'x' HexDigit HexDigit? ;

fragment
HexVariableEscape           : '\\' 'x' '{' HexDigit+ '}' ;

fragment
Unicode2ByteEscape          : '\\' 'u' HexDigit HexDigit HexDigit HexDigit ;

fragment
Unicode4ByteEscape          : '\\' 'U' HexDigit HexDigit HexDigit HexDigit HexDigit HexDigit HexDigit HexDigit ;


// reserved operators

IsEqOperator                : '==' ;
IsLtOperator                : '<' ;
IsLeOperator                : '<=' ;
IsGtOperator                : '>' ;
IsGeOperator                : '>=' ;

AssignOperator              : '=' ;
PlusAssignOperator          : '+=' ;
MinusAssignOperator         : '-=' ;
StarAssignOperator          : '*=' ;
SlashAssignOperator         : '/=' ;

PlusOperator                : '+' ;
MinusOperator               : '-' ;
StarOperator                : '*' ;
SlashOperator               : '/' ;
ArrowOperator               : '->' ;
ColonOperator               : ':' ;
CommaOperator               : ',' ;
SemicolonOperator           : ';' ;
ConvertOperator             : '^:' ;
EllipsisOperator            : '...' ;
DotOperator                 : '.' ;
PipeOperator                : '|' ;
AmpersandOperator           : '&' ;
HashOperator                : '#' ;
DoubleAtOperator            : '@@' ;
AtOperator                  : '@' ;


// reserved keywords

ValKeyword              : 'val' ;
VarKeyword              : 'var' ;
NamedKeyword            : 'named' ;
DefKeyword              : 'def' ;
DeclKeyword             : 'decl' ;
ThisKeyword             : 'this' ;
SuperKeyword            : 'super' ;
GlobalKeyword           : 'global' ;
DefConceptKeyword       : 'defconcept' ;
DefClassKeyword         : 'defclass' ;
DefInstanceKeyword      : 'definstance' ;
DefEnumKeyword          : 'defenum' ;
DefStructKeyword        : 'defstruct' ;
DefAliasKeyword         : 'defalias' ;
NamespaceKeyword        : 'namespace' ;
WhereKeyword            : 'where' ;
SealedKeyword           : 'sealed' ;
FinalKeyword            : 'final' ;
ImplKeyword             : 'impl' ;
ImportKeyword           : 'import' ;
ExportKeyword           : 'export' ;
InitKeyword             : 'init' ;
WithKeyword             : 'with' ;
FromKeyword             : 'from' ;
UsingKeyword            : 'using' ;
LambdaKeyword           : 'lambda' ;
SetKeyword              : 'set' ;
LetKeyword              : 'let' ;
CondKeyword             : 'cond' ;
MatchKeyword            : 'match' ;
DoKeyword               : 'do' ;
WhenKeyword             : 'when' ;
CaseKeyword             : 'case' ;
IfKeyword               : 'if' ;
ThenKeyword             : 'then' ;
ElseKeyword             : 'else' ;
WhileKeyword            : 'while' ;
ForKeyword              : 'for' ;
InKeyword               : 'in' ;
BreakKeyword            : 'break' ;
ContinueKeyword         : 'continue' ;
TryKeyword              : 'try' ;
CatchKeyword            : 'catch' ;
FinallyKeyword          : 'finally' ;
ExpectKeyword           : 'expect' ;
RaiseKeyword            : 'raise' ;
AndKeyword              : 'and' ;
OrKeyword               : 'or' ;
NotKeyword              : 'not' ;
ReturnKeyword           : 'return' ;
TrueKeyword             : 'true' ;
FalseKeyword            : 'false' ;
UndefKeyword            : 'undef' ;
NilKeyword              : 'nil' ;
TypeOfKeyword           : 'typeof' ;
TypeNameKeyword         : 'typename' ;

// an identifier is can be a variable, function, or state symbol

fragment
IdentifierStart         : [a-z] | [A-Z] | '_' ;

fragment
IdentifierChar          : IdentifierStart | [0-9] ;

Identifier              : IdentifierStart IdentifierChar* ;


/** other definitions */

EXPRWS                  : [ \t\r\n]+ -> skip ;  // skip whitespace


// binary literal processing mode

mode BINLITERAL ;

fragment
BinaryDigit                 : ('0'|'1') ;

BinaryInteger               : BinaryDigit+ -> popMode ;
InvalidBinaryLiteral        : ~[ \t\r\n]+ -> popMode ;


// octal literal processing mode

mode OCTLITERAL;

fragment
OctalDigit                  : ('0'..'7') ;

OctalInteger                : OctalDigit+ -> popMode ;
InvalidOctalLiteral         : ~[ \t\r\n]+ -> popMode ;


// decimal literal processing mode

mode EXPONENT;

DecimalExponent             : ('+'|'-')? DecimalDigit+ -> popMode ;
InvalidExponent             : ~[ \t\r\n]+ -> popMode ;


// hexadecimal literal processing mode

mode HEXLITERAL;

fragment
HexDigit                    : ('0'..'9'|'a'..'f'|'A'..'F') ;

fragment
HexExponent                 : ('p'|'P') ('+'|'-')? HexDigit+ ;

HexFloatTrailingPeriod      : HexDigit+ '.' HexDigit* HexExponent? -> popMode ;
HexFloatLeadingPeriod       : '.' HexDigit+ HexExponent? -> popMode ;
HexFloatNoPeriod            : HexDigit+ HexExponent -> popMode ;
HexInteger                  : HexDigit+ -> popMode ;
InvalidHexLiteral           : ~[ \t\r\n]+ -> popMode ;


// comment processing mode

mode COMMENT;
CommentLine             : ( ~[\r\n]+ )? '\r'? '\n' -> popMode ;      // match characters until after the newline
