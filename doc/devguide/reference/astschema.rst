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

:Min Children:              1
:Allowed Children:          Any one of `Expression Classes`_ or `Statement Classes`_

Block evaluates each child in sequence.

``Call``
........

``Case``
........

``Char``
........

:Num Children:              0
:Recognized Properties:     - `LiteralValue`_

Char evaluates a literal character value.

``Cond``
........

``Constraint``
..............

``Ctx``
.......

``Decl``
........

:Num Children:              1
:Allowed Child 1:           `Pack`_

``Def``
.......

:Num Children:              2
:Allowed Child 1:           `Pack`_
:Allowed Child 2:           `Block`_

``DefClass``
............

:Min Children:              0
:Allowed Children:          Any one of `Class Definition Classes`_
:Recognized Properties:     - `AccessEnum`_
                            - `DeriveEnum`_
                            - `GenericOffset`_
                            - `Identifier`_

DefClass defines a new class.

``DefConcept``
..............

:Min Children:              0
:Allowed Children:          Any one of `Concept Definition Classes`_
:Recognized Properties:     - `AccessEnum`_
                            - `DeriveEnum`_
                            - `GenericOffset`_
                            - `Identifier`_

DefConcept defines a new concept.

``DefEnum``
...........

:Min Children:              0
:Allowed Children:          Any one of `Enum Definition Classes`_
:Recognized Properties:     - `AccessEnum`_
                            - `Identifier`_

DefEnum defines a new enumeration.

``DefInstance``
...............

:Min Children:              0
:Allowed Children:          Any one of `Instance Definition Classes`_
:Recognized Properties:     - `AccessEnum`_
                            - `Identifier`_

DefInstance defines a new instance.

``DefStatic``
.............

DefStatic defines a new static.

``DefStruct``
.............

:Min Children:              0
:Allowed Children:          Any one of `Struct Definition Classes`_
:Recognized Properties:     - `AccessEnum`_
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

Unimplemented.

``ExportModule``
................

Unimplemented.

``ExportSymbols``
.................

Unimplemented.

``False``
.........

:Num Children:              0

False evaluates a false boolean value.

``Float``
.........

:Num Children:              0
:Recognized Properties:     - `BaseEnum`_
                            - `LiteralValue`_
                            - `NotationEnum`_

Float evaluates a literal float value.

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

:Num Children:              0
:Recognized Properties:     - `ModuleLocation`_

ImportAll imports all symbols from the import location specified by `ModuleLocation`_ and
inserts each symbol into the current block.

``ImportModule``
................

:Num Children:              0
:Recognized Properties:     - `Identifier`_
                            - `ModuleLocation`_

ImportModule constructs a new namespace with the name specified by `Identifier`_, then imports
all symbols from the import location specified by `ModuleLocation`_ and inserts each symbol
into the new namespace.

``ImportSymbols``
.................

:Min Children:              1
:Allowed Children:          - `SymbolRef`_
:Recognized Properties:     - `ModuleLocation`_

ImportSymbols imports the symbols specified by the `SymbolRef`_ children from the import location
specified by `ModuleLocation`_ and inserts each symbol into the current block.

``Init``
........

``InplaceAdd``
..............

:Num Children:              2
:Allowed Child 1:           - `Name`_
                            - `Target`_
:Allowed Child 2:           Any one of `Expression Classes`_

Set evaluates the value of child 1 (the right operand), applies the addition operator to
the target specified by child 1 (the left operand) and the value, then assigns the result
to the target.

``InplaceDiv``
..............

:Num Children:              2
:Allowed Child 1:           - `Name`_
                            - `Target`_
:Allowed Child 2:           Any one of `Expression Classes`_

Set evaluates the value of child 1 (the right operand), applies the division operator to
the target specified by child 1 (the left operand) and the value, then assigns the result
to the target.

``InplaceMul``
..............

:Num Children:              2
:Allowed Child 1:           - `Name`_
                            - `Target`_
:Allowed Child 2:           Any one of `Expression Classes`_

Set evaluates the value of child 1 (the right operand), applies the multiplication operator
to the target specified by child 1 (the left operand) and the value, then assigns the result
to the target.

``InplaceSub``
..............

:Num Children:              2
:Allowed Child 1:           - `Name`_
                            - `Target`_
:Allowed Child 2:           Any one of `Expression Classes`_

Set evaluates the value of child 1 (the right operand), applies the subtraction operator to
the target specified by child 1 (the left operand) and the value, then assigns the result to
the target.

``Integer``
...........

:Num Children:              0
:Recognized Properties:     - `BaseEnum`_
                            - `LiteralValue`_

Integer evaluates a literal integer value.

``IsA``
.......

:Num Children:              2
:Allowed Child 1:           Any one of `Expression Classes`_
:Allowed Child 1:           Any one of `Type Classes`_

IsA applies the type comparison operator to child 1 (the operand) using the type parameter
specified by child 2.

``IsEq``
........

:Num Children:              2
:Allowed Child 1:           Any one of `Expression Classes`_
:Allowed Child 2:           Any one of `Expression Classes`_

IsEq applies the equals comparision operator to child 1 (the left operand) and child 2
(the right operand).

``IsLe``
........

:Num Children:              2
:Allowed Child 1:           Any one of `Expression Classes`_
:Allowed Child 2:           Any one of `Expression Classes`_

IsLe applies the less-than-or-equals comparision operator to child 1 (the left operand)
and child 2 (the right operand).

``IsLt``
........

:Num Children:              2
:Allowed Child 1:           Any one of `Expression Classes`_
:Allowed Child 2:           Any one of `Expression Classes`_

IsLt applies the less-than comparision operator to child 1 (the left operand) and child 2
(the right operand).

``IsGe``
........

:Num Children:              2
:Allowed Child 1:           Any one of `Expression Classes`_
:Allowed Child 2:           Any one of `Expression Classes`_

IsGe applies the greater-than-or-equal comparision operator to child 1 (the left operand)
and child 2 (the right operand).

``IsGt``
........

:Num Children:              2
:Allowed Child 1:           Any one of `Expression Classes`_
:Allowed Child 2:           Any one of `Expression Classes`_

IsGt applies the greater-than comparision operator to child 1 (the left operand) and child 2
(the right operand).

``IType``
.........

:Min Children:              1
:Allowed Children:          - `PType`_
                            - `SType`_

IType represents an intersection type. An IType must contain at least one member, and each
member must be a PType or a SType.

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

Nil evaluates a nil value.

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

:Min Children:              1
:Allowed Children:          Any one of `Type Classes`_
:Recognized Properties:     - `SymbolPath`_

PType represents a parameterized type. A SType must have a `SymbolPath`_ attribute which specifies the
path to the symbol represented by the type. The children represent the type parameters.

``Rest``
........

``Return``
..........

``Set``
.......

:Num Children:              2
:Allowed Child 1:           - `Name`_
                            - `Target`_
:Allowed Child 2:           Any one of `Expression Classes`_

Set evaluates the value of child 1 (the right operand) and assigns the value to the target specified by
child 1 (the left operand).

``String``
..........

:Num Children:              0
:Recognized Properties:     - `LiteralValue`_

String evaluates a literal string value.

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

:Num Children:              0
:Recognized Properties:     - `SymbolPath`_

String evaluates a reference to the symbol specified by the `SymbolPath`_.

``SType``
.........

:Num Children:              0
:Recognized Properties:     - `SymbolPath`_

SType represents a singular type. A SType must have a `SymbolPath`_ attribute which specifies the
path to the symbol represented by the type.

``Target``
..........

``This``
........

:Num Children:              0

This evaluates a reference to the current receiver.

``True``
........

:Num Children:              0

True evaluates a true boolean value.

``Try``
.......

``TypeArguments``
.................

``Undef``
.........

:Num Children:              0

Undef evaluates an undef value.

``Unpack``
..........

``Using``
.........

``Url``
.......

:Num Children:              0
:Recognized Properties:     - `LiteralValue`_

Url evaluates a literal URL value.

``UType``
.........

:Min Children:              1
:Allowed Children:          - `PType`_
                            - `SType`_

UType represents a union type. A UType must contain at least one member, and each
member must be a PType or a SType.

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

Class Definition Classes
........................

- `Def`_
- `Impl`_
- `Init`_
- `Val`_
- `Var`_

Concept Definition Classes
..........................

- `Decl`_
- `Impl`_

Enum Definition Classes
.......................

- `Case`_
- `Def`_
- `Impl`_
- `Init`_
- `Val`_

Instance Definition Classes
...........................

- `Def`_
- `Impl`_
- `Val`_
- `Var`_

Struct Definition Classes
.........................

- `Def`_
- `Impl`_
- `Init`_
- `Val`_

Type Classes
............

- `IType`_
- `PType`_
- `SType`_
- `UType`_
