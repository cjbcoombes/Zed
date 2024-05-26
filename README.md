# Zed
Zed, a custom programming language compiled to a bytecode and run on a VM. The compiler, vm, assembler, and disassembler are all written in C++

## Why?

***It's interesting.*** I feel like wanting to design their own programming language is a phase many programmers go through. This is that for me. A little bit of creativity, and a project that is mine, top to bottom.

***It's a learning project.*** Before starting, I knew almost nothing about: C++, instruction set architectures, assembly languages, parsing, compiling, linking, or programming language design. Now I know quite a bit, and there's more to come.

***It requires problem solving.*** I love problem solving. And I'm making a deliberate effort to only research the big ideas, so that I can find my own solutions to all of the implementation details.

***It's not meant to be practical.*** I'm sure nothing meets production-level design standards, and there are a lot of mistakes, and it's by no means efficient. That's not the point, and criticism to that effect is moot.

## Overview

### Bytecode/Virtual Machine
The bytecode is a RISC assembly-like language that comes with an assembler, disassembler, and executor (virtual machine). It is deliberately simplistic and tailored to a text-based interface. It supports operations on 32-bit ints, 32-bit floats, 8-bit chars, and 8-bit booleans, as well as standard conditionals and jumps.

Examples are in `Zed/Lang/AssemblyExamples`. For the best examples, see `fibonacci_recursive.azm` (shows use of stack frames) and `guessing_game.azm` (shows user input and number parsing).

### Compiler

#### Steps
1. **Tokenization**: Raw text is parsed into tokens, including literals, identifiers, keywords, and symbols/operators
2. **Pattern Matching**: Tokens are matched into a tree structure using a set of patterns. This step is very lenient, with minimal checking of correctness or type, which forces the patterns (and thus the language) to be generally consistent and unambiguous.
3. **Tree Formation**: The tree of matched patterns is turned into an Abstract Syntax Tree (AST), including symbol resolution and typechecking.
4. **Bytecode Generation**: Work in progress...
5. **Error Reporting**: Errors (at various levels, including warnings and informational messages) are reported, with information on where in the file they came from.