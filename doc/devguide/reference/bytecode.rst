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

:OpInfo Type:               NO_OPERANDS
:Side Effects:              None.

Does nothing.

``NIL``
.......

:OpInfo Type:               NO_OPERANDS
:Side Effects:              1. Value pushed onto data stack

Pushes a `nil` value onto the top of the data stack.

``UNDEF``
.........

:OpInfo Type:               NO_OPERANDS
:Side Effects:              1. Value pushed onto data stack

Pushes an `undef` value onto the top of the data stack.

``TRUE``
........

:OpInfo Type:               NO_OPERANDS
:Side Effects:              1. Value pushed onto data stack

Pushes a `true` value onto the top of the data stack.

``FALSE``
.........

:OpInfo Type:               NO_OPERANDS
:Side Effects:              1. Value pushed onto data stack

Pushes a `false` value onto the top of the data stack.

``I64``
.......

:OpInfo Type:               IMMEDIATE_I64
:Side Effects:              1. Value pushed onto data stack

Pushes a 64-bit integer value onto the top of the data stack. The value is encoded in the 8 subsequent
bytes after the opcode in little-endian byte order.

``DBL``
.......

:OpInfo Type:               IMMEDIATE_DBL
:Side Effects:              1. Value pushed onto data stack

Pushes a double-precision floating point value onto the top of the data stack. The value is encoded in
the 8 subsequent bytes after the opcode in IEEE 754 binary64 format.

``CHR``
.......

:OpInfo Type:               IMMEDIATE_CHR
:Side Effects:              1. Value pushed onto data stack

Pushes a 32-bit Unicode code point value onto the top of the data stack. The value is encoded in the 4
subsequent bytes after the opcode in little-endian byte order.

``LITERAL``
...........

:OpInfo Type:               ADDRESS_U32
:Address Type:              Literal Address
:Side Effects:              1. Value pushed onto data stack


``STRING``
..........

:OpInfo Type:               ADDRESS_U32
:Address Type:              Literal Address
:Side Effects:              1. Value pushed onto data stack

``URL``
.......

:OpInfo Type:               ADDRESS_U32
:Address Type:              Literal Address
:Side Effects:              1. Value pushed onto data stack

``SYNTHETIC``
.............

:OpInfo Type:               TYPE_U8
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

:OpInfo Type:               ADDRESS_U32
:Address Type:              Literal Address
:Side Effects:              1. Value pushed onto data stack

``LOAD``
........

:OpInfo Type:               FLAGS_U8_ADDRESS_U32
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

:OpInfo Type:               FLAGS_U8_ADDRESS_U32
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

:OpInfo Type:               NO_OPERANDS
:Side Effects:              1. Value popped off of data stack

``DUP``
.......

:OpInfo Type:               NO_OPERANDS
:Side Effects:              1. Value pushed onto data stack

``PICK``
........

:OpInfo Type:               OFFSET_U16
:Side Effects:              1. Value pushed onto data stack

``DROP``
........

:OpInfo Type:               OFFSET_U16
:Side Effects:              1. Value removed from data stack

``RPICK``
.........

:OpInfo Type:               OFFSET_U16
:Side Effects:              1. Value pushed onto data stack

``RDROP``
.........

:OpInfo Type:               OFFSET_U16
:Side Effects:              1. Value removed from data stack

``I64_ADD``
...........

:OpInfo Type:               NO_OPERANDS
:Preconditions:             1. Int64 value (the `rhs`) is on the top of the stack
                            2. Int64 value (the `lhs`) is directly below the top of the stack
:Side Effects:              1. `rhs` value popped off data stack
                            2. `lhs` value popped off data stack
                            3. `result` value pushed onto data stack

Pops the `rhs` and `lhs` values off the data stack, performs the operation :code:`lhs + rhs`, and pushes the
result onto the data stack.

``I64_SUB``
...........

:OpInfo Type:               NO_OPERANDS
:Preconditions:             1. Int64 value (the `rhs`) is on the top of the stack
                            2. Int64 value (the `lhs`) is directly below the top of the stack
:Side Effects:              1. `rhs` value popped off data stack
                            2. `lhs` value popped off data stack
                            3. `result` value pushed onto data stack

Pops the `rhs` and `lhs` values off the data stack, performs the operation :code:`lhs - rhs`, and pushes the
result onto the data stack.

``I64_MUL``
...........

:OpInfo Type:               NO_OPERANDS
:Preconditions:             1. Int64 value (the `rhs`) is on the top of the stack
                            2. Int64 value (the `lhs`) is directly below the top of the stack
:Side Effects:              1. `rhs` value popped off data stack
                            2. `lhs` value popped off data stack
                            3. `result` value pushed onto data stack

Pops the `rhs` and `lhs` values off the data stack, performs the operation :code:`lhs * rhs`, and pushes the
result onto the data stack.

``I64_DIV``
...........

:OpInfo Type:               NO_OPERANDS
:Preconditions:             1. Int64 value (the `rhs`) is on the top of the stack
                            2. Int64 value (the `lhs`) is directly below the top of the stack
:Side Effects:              1. `rhs` value popped off data stack
                            2. `lhs` value popped off data stack
                            3. `result` value pushed onto data stack

Pops the `rhs` and `lhs` values off the data stack, performs the operation :code:`lhs / rhs`, and pushes the
result onto the data stack.

``I64_NEG``
...........

:OpInfo Type:               NO_OPERANDS
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

``CALL_VIRTUAL``
................

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
