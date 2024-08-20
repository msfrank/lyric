==========
AST Schema
==========

.. contents:: Table of Contents
   :depth: 2

The AST Schema defines the elements of a Lyric abstract syntax tree.

:Schema Namespace URL:          ``dev.zuri.ns:ast-1``

AST Classes
-----------
.. contents::
   :local:

``Add``
.......

:Num Children:              2
:Allowed Child 1:           Any one of `Expression Classes`_
:Allowed Child 2:           Any one of `Expression Classes`_

Add applies the addition operator to child 1 (the left operand) and child 2 (the right
operand).

``And``
.......

:Num Children:              2
:Allowed Child 1:           Any one of `Expression Classes`_
:Allowed Child 2:           Any one of `Expression Classes`_

And applies the logical conjunction operator to child 1 (the left operand) and child 2
(the right operand).

``Block``
.........

``Call``
........

``Case``
........

``Char``
........

:Num Children:              0
:Recognized Attributes:     - `LiteralValue`_

Declares a literal character value.

``Cond``
........

``Constraint``
..............

``Ctx``
.......

``Def``
.......


``DefClass``
............

:Min Children:              0
:Max Children:              N/A
:Recognized Attributes:     - `AccessEnum`_
                            - `DeriveEnum`_
                            - `GenericOffset`_
                            - `Identifier`_

DefClass defines a new class.

``DefConcept``
..............

:Min Children:              0
:Max Children:              N/A
:Recognized Attributes:     - `AccessEnum`_
                            - `DeriveEnum`_
                            - `GenericOffset`_
                            - `Identifier`_

DefConcept defines a new concept.

``DefEnum``
...........

:Min Children:              0
:Max Children:              N/A
:Recognized Attributes:     - `AccessEnum`_
                            - `Identifier`_

DefEnum defines a new enumeration.

``DefInstance``
...............

:Min Children:              0
:Max Children:              N/A
:Recognized Attributes:     - `AccessEnum`_
                            - `Identifier`_

DefInstance defines a new instance.

``DefStatic``
.............

DefStruct defines a new static.

``DefStruct``
.............

:Min Children:              0
:Max Children:              N/A
:Recognized Attributes:     - `AccessEnum`_
                            - `DeriveEnum`_
                            - `Identifier`_

DefStruct defines a new struct.

``Deref``
.........

``Div``
.......

:Num Children:              2
:Allowed Child 1:           Any one of `Expression Classes`_
:Allowed Child 2:           Any one of `Expression Classes`_

Div applies the division operator to child 1 (the left operand) and child 2 (the right operand).

``ExportAll``
.............

``ExportModule``
................

``ExportSymbols``
.................

``False``
.........

:Num Children:              0
:Recognized Attributes:     - `LiteralValue`_

Declares a false boolean value.

``Float``
.........

:Num Children:              0
:Recognized Attributes:     - `LiteralValue`_

Declares a literal float value.

``For``
.......

``Generic``
...........

``If``
......

``Impl``
........

``ImportAll``
.............

``ImportModule``
................

``ImportSymbols``
.................

``Init``
........

``InplaceAdd``
..............

``InplaceDiv``
..............

``InplaceMul``
..............

``InplaceSub``
..............

``Integer``
...........

:Num Children:              0
:Recognized Attributes:     - `LiteralValue`_

Declares a literal integer value.

``IsA``
.......

``IsEq``
........

``IsLe``
........

``IsLt``
........

``IsGe``
........

``IsGt``
........

``IType``
.........

``Keyword``
...........

``Lambda``
..........

``MacroCall``
.............

``MacroList``
.............

``Match``
.........

``Mul``
.......

:Num Children:              2
:Allowed Child 1:           Any one of `Expression Classes`_
:Allowed Child 2:           Any one of `Expression Classes`_

Mul applies the multiplication operator to child 1 (the left operand) and child 2 (the
right operand).

``Name``
........

``Namespace``
.............

``Neg``
.......

:Num Children:              1
:Allowed Child 1:           Any one of `Expression Classes`_

Neg applies the additive inverse operator to child 1 (the operand).

``New``
.......

``Nil``
.......

:Num Children:              0
:Recognized Attributes:     - `LiteralValue`_

Declares a nil value.

``Not``
.......

:Num Children:              1
:Allowed Child 1:           Any one of `Expression Classes`_

Not applies the logical complement operator to child 1 (the operand).

``Or``
......

:Num Children:              2
:Allowed Child 1:           Any one of `Expression Classes`_
:Allowed Child 2:           Any one of `Expression Classes`_

Or applies the logical disjunction operator to child 1 (the left operand) and child 2
(the right operand).

``Pack``
........

``Pair``
........

``Param``
.........

``Placeholder``
...............

``PType``
.........

``Rest``
........

``Return``
..........

``Set``
.......

``String``
..........

:Num Children:              0
:Recognized Attributes:     - `LiteralValue`_

Declares a literal string value.

``Sub``
.......

:Num Children:              2
:Allowed Child 1:           Any one of `Expression Classes`_
:Allowed Child 2:           Any one of `Expression Classes`_

Sub applies the subtraction operator to child 1 (the left operand) and child 2 (the right operand).

``Super``
.........

``SymbolRef``
.............

``SType``
.........

``Target``
..........

``This``
........

``True``
........

:Num Children:              0
:Recognized Attributes:     - `LiteralValue`_

Declares a true boolean value.

``Try``
.......

``TypeArguments``
.................

``Undef``
.........

:Num Children:              0
:Recognized Attributes:     - `LiteralValue`_

Declares an undef value.

``Unpack``
..........

``Using``
.........

``Url``
.......

:Num Children:              0
:Recognized Attributes:     - `LiteralValue`_

Declares a literal URL value.

``UType``
.........

``Val``
.......

``Var``
.......

``When``
........

``While``
.........


AST Properties
--------------
.. contents::
   :local:

``AccessEnum``
..............

:Property Type:             UInt32
:Allowed Values:
   .. table::
      :align: left

      =====  =====
      Index  Value
      =====  =====
      0      Public
      1      Protected
      2      Private
      =====  =====

Access level enumeration encoded as a uint32.

``BaseEnum``
............

:Property Type:             UInt32
:Allowed Values:
   .. table::
      :align: left

      =====  =====
      Index  Value
      =====  =====
      0      Binary
      1      Octal
      2      Decimal
      3      Hex
      =====  =====

Number radix enumeration encoded as a uint32.

``BoundEnum``
.............

:Property Type:             UInt32
:Allowed Values:
   .. table::
      :align: left

      =====  =====
      Index  Value
      =====  =====
      0      None
      1      Extends
      2      Super
      =====  =====

Type bound enumeration encoded as a uint32.

``DefaultOffset``
.................

:Property Type:             UInt32


``DeriveEnum``
..............

:Property Type:             UInt32
:Allowed Values:
   .. table::
      :align: left

      =====  =====
      Index  Value
      =====  =====
      0      Any
      1      Sealed
      2      Final
      =====  =====

Derive enumeration encoded as a uint32.

``FinallyOffset``
.................

:Property Type:             UInt32



``GenericOffset``
.................

:Property Type:             UInt32


``Identifier``
..............

:Property Type:             String

A symbol identifier.

``LiteralValue``
................

:Property Type:             String


``IsVariable``
..............

:Property Type:             Bool

``Label``
.........

:Property Type:             String

``MacroListOffset``
...................

:Property Type:             UInt32

``ModuleLocation``
..................

:Property Type:             String

``NotationEnum``
................

:Property Type:             UInt32
:Allowed Values:
   .. table::
      :align: left

      =====  =====
      Index  Value
      =====  =====
      0      Fixed
      1      Scientific
      =====  =====

Floating-point notation enumeration encoded as a uint32.

``RestOffset``
..............

:Property Type:             UInt32

``SymbolPath``
..............

:Property Type:             String

``SymbolUrl``
.............

:Property Type:             String

``TypeArgumentsOffset``
.......................

:Property Type:             UInt32

``TypeOffset``
..............

:Property Type:             UInt32

``VarianceEnum``
................

:Property Type:             UInt32
:Allowed Values:
   .. table::
      :align: left

      =====  =====
      Index  Value
      =====  =====
      0      Invariant
      1      Covariant
      2      Contravariant
      =====  =====

Variance enumeration encoded as a uint32.

AST Classes By Section
----------------------

Literal Classes
...............

- `Char`_
- `False`_
- `Float`_
- `Integer`_
- `Nil`_
- `Pair`_
- `String`_
- `SymbolRef`_
- `True`_
- `Undef`_
- `Url`_

Arithmetic Classes
..................

- `Add`_
- `Div`_
- `Mul`_
- `Neg`_
- `Sub`_

Comparison Classes
..................

- `IsA`_
- `IsEq`_
- `IsLe`_
- `IsLt`_
- `IsGe`_
- `IsGt`_

Logical Classes
...............

- `And`_
- `Not`_
- `Or`_

Type Classes
............

- `IType`_
- `PType`_
- `SType`_
- `TypeArguments`_
- `UType`_

Assignment Classes
..................

- `InplaceAdd`_
- `InplaceSub`_
- `InplaceMul`_
- `InplaceDiv`_
- `Set`_
- `Target`_

Dereference Classes
...................

- `Call`_
- `Deref`_
- `Name`_
- `This`_

Construction Classes
....................

- `Lambda`_
- `New`_

Control Classes
...............

- `Block`_
- `Cond`_
- `For`_
- `If`_
- `Match`_
- `Try`_
- `Return`_
- `When`_
- `While`_

Definition Classes
..................

- `Case`_
- `Def`_
- `DefClass`_
- `DefConcept`_
- `DefEnum`_
- `DefInstance`_
- `DefStatic`_
- `DefStruct`_
- `Impl`_
- `Namespace`_
- `Val`_
- `Var`_

Initialization Classes
......................

- `Init`_
- `Super`_

Import Classes
..............

- `ImportAll`_
- `ImportModule`_
- `ImportSymbols`_

Export Classes
..............

- `ExportAll`_
- `ExportModule`_
- `ExportSymbols`_

Macro Classes
.............

- `MacroCall`_
- `MacroList`_

Miscellaneous Classes
.....................

- `Constraint`_
- `Ctx`_
- `Generic`_
- `Keyword`_
- `Pack`_
- `Param`_
- `Placeholder`_
- `Rest`_
- `Unpack`_
- `Using`_

Expression Classes
..................

- `Add`_
- `And`_
- `Block`_
- `Call`_
- `Char`_
- `Cond`_
- `Deref`_
- `Div`_
- `False`_
- `Float`_
- `Integer`_
- `IsA`_
- `IsEq`_
- `IsGe`_
- `IsGt`_
- `IsLe`_
- `IsLt`_
- `Lambda`_
- `Match`_
- `Mul`_
- `Name`_
- `Neg`_
- `New`_
- `Nil`_
- `Not`_
- `Or`_
- `String`_
- `Sub`_
- `SymbolRef`_
- `This`_
- `True`_
- `Undef`_
- `Url`_

Statement Classes
.................

- `Def`_
- `DefClass`_
- `DefConcept`_
- `DefEnum`_
- `DefInstance`_
- `DefStatic`_
- `DefStruct`_
- `For`_
- `Generic`_
- `If`_
- `ImportAll`_
- `ImportModule`_
- `ImportSymbols`_
- `Init`_
- `InplaceAdd`_
- `InplaceDiv`_
- `InplaceMul`_
- `InplaceSub`_
- `Namespace`_
- `Return`_
- `Set`_
- `Using`_
- `Val`_
- `Var`_
- `While`_
