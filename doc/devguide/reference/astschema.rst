==========
AST Schema
==========

:Schema Namespace:          ``dev.zuri.ns:ast-1``

AST Classes
-----------
.. contents::
   :local:

``Add``
.......

``And``
.......

``Block``
.........

``Call``
........

``Case``
........

``Char``
........

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

``ExportAll``
.............

``ExportModule``
................

``ExportSymbols``
.................

``False``
.........

``Float``
.........

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

``Name``
........

``Namespace``
.............

``Neg``
.......

``New``
.......

``Nil``
.......

``Not``
.......

``Or``
......

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

``Sub``
.......

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

``Try``
.......

``TypeArguments``
.................

``Undef``
.........

``Unpack``
..........

``Using``
.........

``Url``
.......

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

- `Nil`_
- `Undef`_
- `False`_
- `True`_
- `Integer`_
- `Float`_
- `Char`_
- `String`_
- `Url`_
- `Pair`_

Arithmetic Classes
..................

- `Add`_
- `Sub`_
- `Mul`_
- `Div`_
- `Neg`_

Comparison Classes
..................

- `IsEq`_
- `IsLt`_
- `IsLe`_
- `IsGt`_
- `IsGe`_
- `IsA`_

Logical Classes
...............

- `And`_
- `Or`_
- `Not`_

Type Classes
............

- `SType`_
- `PType`_
- `IType`_
- `UType`_
- `TypeArguments`_

Assignment Classes
..................

- `Set`_
- `Target`_
- `InplaceAdd`_
- `InplaceSub`_
- `InplaceMul`_
- `InplaceDiv`_

Dereference Classes
...................

- `Deref`_
- `This`_
- `Name`_
- `Call`_

Construction Classes
....................

- `New`_
- `Lambda`_

Control Classes
...............

- `Block`_
- `If`_
- `Cond`_
- `Match`_
- `When`_
- `While`_
- `For`_
- `Try`_
- `Return`_

Definition Classes
..................

- `Def`_
- `DefClass`_
- `DefConcept`_
- `DefEnum`_
- `DefInstance`_
- `DefStruct`_
- `Namespace`_
- `Val`_
- `Var`_
- `Case`_
- `Impl`_

Initialization Classes
......................

- `Super`_
- `Init`_

Import Classes
..............

- `ImportAll`_
- `ImportSymbols`_
- `ImportModule`_

Import Classes
..............

- `ExportAll`_
- `ExportSymbols`_
- `ExportModule`_
- `Using`_
- `SymbolRef`_

Macro Classes
.............

- `MacroCall`_
- `MacroList`_

Miscellaneous Classes
.....................

- `Unpack`_
- `Pack`_
- `Param`_
- `Keyword`_
- `Rest`_
- `Ctx`_
- `Generic`_
- `Placeholder`_
- `Constraint`_