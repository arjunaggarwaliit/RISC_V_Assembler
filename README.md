# RISC-V Assembler (64-bit)

## Overview

This repository implements a **64-bit RISC-V assembler** that converts RISC-V assembly code (`.asm`) into corresponding machine code (`.mc`), similar in spirit to the **Venus assembler**. The project is developed as part of the **CS207 – Foundations of Computer Systems (FCS) Lab** course project.

The assembler parses RISC-V assembly instructions, resolves labels and directives, and emits machine code along with human-readable metadata for debugging and verification.

---

## Key Features

* Supports **RV64 ISA (subset of 37 instructions)**
* Handles **multiple instruction formats**: R, I, S, SB, U, and UJ
* Implements **assembler directives** for text and data segments
* Resolves **labels** using multi-pass parsing
* Produces output compatible with **Venus-style `.mc` format**
* Written with clarity and modularity to aid learning and extensibility

---

## Instruction Set Supported

### R-Type Instructions

```
add, addw, and, or, sll, slt, sra, srl, sub, subw, xor,
mul, mulw, div, divw, rem, remw
```

### I-Type Instructions

```
addi, addiw, andi, ori, lb, ld, lh, lw, jalr
```

### S-Type Instructions

```
sb, sh, sw, sd
```

### SB-Type (Branch) Instructions

```
beq, bne, bge, blt
```

### U-Type Instructions

```
lui, auipc
```

### UJ-Type Instructions

```
jal
```

> ⚠️ Pseudo-instructions and floating-point instructions are **not supported**.

---

## Assembler Directives Supported

| Directive | Description                            |
| --------- | -------------------------------------- |
| `.text`   | Marks beginning of text (code) segment |
| `.data`   | Marks beginning of data segment        |
| `.byte`   | 1-byte data                            |
| `.half`   | 2-byte data                            |
| `.word`   | 4-byte data                            |
| `.dword`  | 8-byte data                            |
| `.asciz`  | Null-terminated string                 |

---

## Memory Layout Assumptions

The assembler follows these fixed memory segment base addresses (as per project specification):

| Segment | Base Address |
| ------- | ------------ |
| Text    | `0x00000000` |
| Data    | `0x10000000` |
| Heap    | `0x10008000` |
| Stack   | `0x7FFFFFFC` |

---

## Input Format (`input.asm`)

* One instruction per line
* Standard RISC-V register naming (`x0`–`x31`)
* Labels supported

### Example

```asm
.text
add x1, x2, x3
andi x5, x6, 10
```

---

## Output Format (`output.mc`)

Each line in the output file follows the format:

```
<address> <machine_code> , <assembly_instruction> # <decoded_fields>
```

### Example

```
0x0 0x003100B3 , add x1,x2,x3 # 0110011-000-0000000-00001-00010-00011-NULL
0x4 0x00A37293 , andi x5,x6,10 # 0010011-111-NULL-00101-00110-000000001010
```

A termination marker is emitted at the end of the text segment to indicate program completion.

---

## Design & Architecture

### High-Level Flow

1. **First Pass**

   * Parse `.asm` file
   * Identify labels and their addresses
   * Track text and data segment sizes

2. **Second Pass**

   * Encode instructions into machine code
   * Resolve label references (branches, jumps)
   * Generate `.mc` output

---

### Major Components

* **Lexer / Parser**

  * Tokenizes instructions, registers, immediates, and labels

* **Instruction Encoder**

  * Encodes fields like opcode, funct3, funct7, rd, rs1, rs2, immediates

* **Symbol Table**

  * Stores label → address mappings

* **Directive Handler**

  * Manages `.text`, `.data`, and data allocation directives

* **Output Formatter**

  * Produces Venus-style `.mc` lines with annotations

---

## Register Encoding

Registers are mapped according to the RISC-V specification:

```
x0  → 00000   x16 → 10000
x1  → 00001   ...
x31 → 11111
```

---

## Immediate Handling

* Sign-extension handled as per instruction format
* Branch and jump offsets computed relative to PC
* Alignment constraints enforced

---

## How to Build & Run

### Compilation (C++ example)

```bash
g++ -std=c++17 assembler.cpp -o assembler
```

### Execution

```bash
./assembler input.asm output.mc
```

> Ensure `input.asm` is present in the working directory or provide correct paths.

---

## Error Handling

The assembler reports errors for:

* Invalid instruction mnemonics
* Incorrect operand counts
* Invalid register names
* Undefined labels
* Immediate overflow
* Misaligned branch/jump targets

---

## Limitations

* No pseudo-instruction expansion
* No floating-point or CSR instructions
* Fixed memory layout (non-configurable)

---

## Future Improvements

* Add pseudo-instruction support (`li`, `mv`, `nop`, etc.)
* Implement floating-point and CSR instructions
* Improve diagnostics with line numbers
* Support ELF output format

---

## Academic Context

This project was completed as part of **CS207 – FCS Lab** at **IIT Ropar**, with the goal of understanding instruction encoding, assembly pipelines, and system-level programming concepts.

---

## Authors

* **Arjun Aggarwal** 

---

## License

This project is intended for **academic use only**. Please consult the course policy before reuse or redistribution.
