===============
Object Bytecode
===============

.. contents:: Table of Contents
   :depth: 2

The object bytecode is the binary-coded instruction set which is executed on the Lyric virtual machine.

Opcodes
-------
.. contents::
   :local:

``NOOP``
........

:OpInfo Type:               `NO_OPERANDS`_
:Side Effects:              None.

Does nothing.

``NIL``
.......

:OpInfo Type:               `NO_OPERANDS`_
:Side Effects:              1. Value pushed onto data stack

Pushes a `nil` value onto the top of the data stack.

``UNDEF``
.........

:OpInfo Type:               `NO_OPERANDS`_
:Side Effects:              1. Value pushed onto data stack

Pushes an `undef` value onto the top of the data stack.

``TRUE``
........

:OpInfo Type:               `NO_OPERANDS`_
:Side Effects:              1. Value pushed onto data stack

Pushes a `true` value onto the top of the data stack.

``FALSE``
.........

:OpInfo Type:               `NO_OPERANDS`_
:Side Effects:              1. Value pushed onto data stack

Pushes a `false` value onto the top of the data stack.

``I64``
.......

:OpInfo Type:               `IMMEDIATE_I64`_
:Side Effects:              1. Value pushed onto data stack

Pushes a 64-bit integer value onto the top of the data stack. The value is encoded in the 8 subsequent
bytes after the opcode in little-endian byte order.

``DBL``
.......

:OpInfo Type:               `IMMEDIATE_DBL`_
:Side Effects:              1. Value pushed onto data stack

Pushes a double-precision floating point value onto the top of the data stack. The value is encoded in
the 8 subsequent bytes after the opcode in IEEE 754 binary64 format.

``CHR``
.......

:OpInfo Type:               `IMMEDIATE_CHR`_
:Side Effects:              1. Value pushed onto data stack

Pushes a 32-bit Unicode code point value onto the top of the data stack. The value is encoded in the 4
subsequent bytes after the opcode in little-endian byte order.

``LITERAL``
...........

:OpInfo Type:               `ADDRESS_U32`_
:Address Type:              Literal Address
:Side Effects:              1. Value pushed onto data stack


``STRING``
..........

:OpInfo Type:               `ADDRESS_U32`_
:Address Type:              Literal Address
:Side Effects:              1. Value pushed onto data stack

``URL``
.......

:OpInfo Type:               `ADDRESS_U32`_
:Address Type:              Literal Address
:Side Effects:              1. Value pushed onto data stack

``SYNTHETIC``
.............

:OpInfo Type:               `TYPE_U8`_
:Type Allowed Values:
   .. table::
      :align: left

      =====  =====
      Index  Value
      =====  =====
      1      This
      =====  =====
:Side Effects:              1. Value pushed onto data stack

``DESCRIPTOR``
..............

:OpInfo Type:               `ADDRESS_U32`_
:Address Type:              Literal Address
:Side Effects:              1. Value pushed onto data stack

``LOAD``
........

:OpInfo Type:               `FLAGS_U8_ADDRESS_U32`_
:Flags Allowed Values:
   .. table::
      :align: left

      =====  =====
      Index  Value
      =====  =====
      1      Argument
      2      Local
      3      Lexical
      4      Field
      5      Static
      6      Instance
      7      Enum
      =====  =====
:Address Type:              Load Address
:Side Effects:              1. Value pushed onto data stack

``STORE``
.........

:OpInfo Type:               `FLAGS_U8_ADDRESS_U32`_
:Flags Allowed Values:
   .. table::
      :align: left

      =====  =====
      Index  Value
      =====  =====
      1      Argument
      2      Local
      3      Lexical
      4      Field
      5      Static
      =====  =====
:Address Type:              Store Address
:Side Effects:              1. Value popped off of data stack

``VA_LOAD``
...........

``VA_SIZE``
...........

``POP``
.......

:OpInfo Type:               `NO_OPERANDS`_
:Side Effects:              1. Value popped off of data stack

``DUP``
.......

:OpInfo Type:               `NO_OPERANDS`_
:Side Effects:              1. Value pushed onto data stack

``PICK``
........

:OpInfo Type:               `OFFSET_U16`_
:Side Effects:              1. Value pushed onto data stack

``DROP``
........

:OpInfo Type:               `OFFSET_U16`_
:Side Effects:              1. Value removed from data stack

``RPICK``
.........

:OpInfo Type:               `OFFSET_U16`_
:Side Effects:              1. Value pushed onto data stack

``RDROP``
.........

:OpInfo Type:               `OFFSET_U16`_
:Side Effects:              1. Value removed from data stack

``I64_ADD``
...........

:OpInfo Type:               `NO_OPERANDS`_
:Preconditions:             1. Int64 value (the `rhs`) is on the top of the stack
                            2. Int64 value (the `lhs`) is directly below the top of the stack
:Side Effects:              1. `rhs` value popped off data stack
                            2. `lhs` value popped off data stack
                            3. `result` value pushed onto data stack

Pops the `rhs` and `lhs` values off the data stack, performs the operation :code:`lhs + rhs`, and pushes the
result onto the data stack.

``I64_SUB``
...........

:OpInfo Type:               `NO_OPERANDS`_
:Preconditions:             1. Int64 value (the `rhs`) is on the top of the stack
                            2. Int64 value (the `lhs`) is directly below the top of the stack
:Side Effects:              1. `rhs` value popped off data stack
                            2. `lhs` value popped off data stack
                            3. `result` value pushed onto data stack

Pops the `rhs` and `lhs` values off the data stack, performs the operation :code:`lhs - rhs`, and pushes the
result onto the data stack.

``I64_MUL``
...........

:OpInfo Type:               `NO_OPERANDS`_
:Preconditions:             1. Int64 value (the `rhs`) is on the top of the stack
                            2. Int64 value (the `lhs`) is directly below the top of the stack
:Side Effects:              1. `rhs` value popped off data stack
                            2. `lhs` value popped off data stack
                            3. `result` value pushed onto data stack

Pops the `rhs` and `lhs` values off the data stack, performs the operation :code:`lhs * rhs`, and pushes the
result onto the data stack.

``I64_DIV``
...........

:OpInfo Type:               `NO_OPERANDS`_
:Preconditions:             1. Int64 value (the `rhs`) is on the top of the stack
                            2. Int64 value (the `lhs`) is directly below the top of the stack
:Side Effects:              1. `rhs` value popped off data stack
                            2. `lhs` value popped off data stack
                            3. `result` value pushed onto data stack

Pops the `rhs` and `lhs` values off the data stack, performs the operation :code:`lhs / rhs`, and pushes the
result onto the data stack.

``I64_NEG``
...........

:OpInfo Type:               `NO_OPERANDS`_
:Preconditions:             1. Int64 value (the `operand`) is on the top of the stack
:Side Effects:              1. `operand` value popped off data stack
                            3. `result` value pushed onto data stack

Pops the `operand` value off the data stack, performs the operation :code:`0 - operand`, and pushes the
result onto the data stack.

``DBL_ADD``
...........

``DBL_SUB``
...........

``DBL_MUL``
...........

``DBL_DIV``
...........

``DBL_NEG``
...........

``BOOL_CMP``
............

``I64_CMP``
...........

``DBL_CMP``
...........

``CHR_CMP``
...........

``TYPE_CMP``
............

``LOGICAL_AND``
...............

``LOGICAL_OR``
..............

``LOGICAL_NOT``
...............

``IF_NIL``
..........

``IF_NOTNIL``
.............

``IF_TRUE``
...........

``IF_FALSE``
............

``IF_ZERO``
...........

``IF_NOTZERO``
..............

``IF_GT``
.........

``IF_GE``
.........

``IF_LT``
.........

``IF_LE``
.........

``JUMP``
........

``IMPORT``
..........

``CALL_STATIC``
...............

:OpInfo Type:               `FLAGS_U8_ADDRESS_U32_PLACEMENT_U16`_
:Flags Allowed Values:
   .. table::
      :align: left

      =====  =====
      Index  Value
      =====  =====
      1-255  Reserved for future expansion
      =====  =====
:Address Type:              Call Address
:Preconditions:             None
:Side Effects:              1. <placement> elements are popped from the top of the data stack on the current coroutine
                            2. Instruction Pointer (IP) for the current coroutine is set to the beginning of the
                               callee proc
                            3. Segment Pointer (SP) for the current coroutine is set to the segment containing the
                               callee
                            4. Call cell is allocated on the top of the call stack on the current coroutine


``CALL_VIRTUAL``
................
:OpInfo Type:               `FLAGS_U8_ADDRESS_U32_PLACEMENT_U16`_
:Flags Allowed Values:
   .. table::
      :align: left

      =====  =====
      Index  Value
      =====  =====
      1      RECEIVER_FOLLOWS
      2      FORWARD_REST
      =====  =====
:Address Type:              Call Address
:Preconditions:             None
:Side Effects:              1. if RECEIVER_FOLLOWS is set then receiver cell is popped from the top of the data stack
                               on the current coroutine,
                            2. <placement> elements are popped from the top of the data stack on the current coroutine
                            3. Instruction Pointer (IP) for the current coroutine is set to the beginning of the
                               callee proc
                            4. Segment Pointer (SP) for the current coroutine is set to the segment containing the
                               callee
                            5. Call cell is allocated on the top of the call stack on the current coroutine

``CALL_CONCEPT``
................

``CALL_EXISTENTIAL``
....................

``TRAP``
........

``RETURN``
..........

``NEW``
.......

``TYPE_OF``
...........

``INTERRUPT``
.............

``HALT``
........

``ABORT``
.........

Opcodes By OpInfo Type
----------------------

NO_OPERANDS
...........

- `NOOP`_
- `NIL`_
- `UNDEF`_
- `TRUE`_
- `FALSE`_
- `VA_LOAD`_
- `VA_SIZE`_
- `POP`_
- `DUP`_
- `I64_ADD`_
- `I64_SUB`_
- `I64_MUL`_
- `I64_DIV`_
- `I64_NEG`_
- `DBL_ADD`_
- `DBL_SUB`_
- `DBL_MUL`_
- `DBL_DIV`_
- `DBL_NEG`_
- `BOOL_CMP`_
- `I64_CMP`_
- `DBL_CMP`_
- `CHR_CMP`_
- `TYPE_CMP`_
- `LOGICAL_AND`_
- `LOGICAL_OR`_
- `LOGICAL_NOT`_
- `RETURN`_
- `TYPE_OF`_
- `INTERRUPT`_
- `HALT`_
- `ABORT`_

ADDRESS_U32
...........

- `LITERAL`_
- `STRING`_
- `URL`_
- `IMPORT`_

FLAGS_U8_ADDRESS_U32
....................

- `DESCRIPTOR`_
- `LOAD`_
- `STORE`_
- `TRAP`_

JUMP_I16
........

- `IF_NIL`_
- `IF_NOTNIL`_
- `IF_TRUE`_
- `IF_FALSE`_
- `IF_ZERO`_
- `IF_NOTZERO`_
- `IF_GT`_
- `IF_GE`_
- `IF_LT`_
- `IF_LE`_
- `JUMP`_

OFFSET_U16
..........

- `PICK`_
- `DROP`_
- `RPICK`_
- `RDROP`_

FLAGS_U8_ADDRESS_U32_PLACEMENT_U16
..................................

- `CALL_STATIC`_
- `CALL_VIRTUAL`_
- `CALL_CONCEPT`_
- `CALL_EXISTENTIAL`_
- `NEW`_

FLAGS_U8_OFFSET_U16_PLACEMENT_U16
.................................

TYPE_U8
.......

- `SYNTHETIC`_

IMMEDIATE_I64
.............

- `I64`_

IMMEDIATE_DBL
.............

- `DBL`_

IMMEDIATE_CHR
.............

- `CHR`_