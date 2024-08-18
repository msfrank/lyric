==========
AST Schema
==========

:Schema Namespace:          ``dev.zuri.ns:ast-1``

AST Classes
-----------

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

AST Properties
--------------

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
