# Minimal RV32I LLVM Backend

This is an educational out-of-tree LLVM backend for the RISC-V 32-bit Integer Base (RV32I) architecture. 

It demonstrates how to construct a functioning LLVM backend from scratch by implementing the essential TableGen definitions and C++ classes required to parse LLVM IR and emit valid, executable RISC-V machine code.

## Features Supported
*   **Instruction Selection**: Lowers LLVM IR into RV32I machine instructions (Arithmetic, Memory, Branches, and Calls).
*   **Target Machine Setup**: Defines the Subtarget, TargetInfo, and TargetMachine classes required to register an out-of-tree architecture.
*   **Calling Convention**: Implements a minimal `ilp32` ABI calling convention (passing arguments in `a0-a7`).
*   **Instruction Printing & Assembly Emitting**: Lowers MCInst to object files/assembly strings, complete with handling of `%hi` and `%lo` modifiers for global address resolution.
*   **Bare-metal Executable Generation**: Capable of producing static binaries that run on the standard Spike RISC-V ISA simulator.

## Project Structure
*   `CMakeLists.txt`: Build system configuration to integrate the out-of-tree backend with an existing LLVM installation.
*   `*.td` (TableGen files): Define the RV32I register file (`RV32IRegisterInfo.td`), instruction formats (`RV32IInstrFormats.td`), target instructions (`RV32IInstrInfo.td`), and calling conventions (`RV32ICallingConv.td`).
*   `RV32ISelLowering.cpp`: The core of the backend's ABI compatibility. Implements `LowerFormalArguments`, `LowerCall`, and `LowerReturn`, as well as Custom lowering logic for operations like branches (`BRCOND` and `BR_CC`) and globals.
*   `RV32ISelDAGtoDAG.cpp`: Defines custom instruction selection logic (e.g., converting `FrameIndex` into `ADDI`).
*   `driver/compiler.cpp`: A custom `rv32i-compiler` driver tool that acts like `llc`. It parses LLVM IR files, loads the custom backend dynamically, and outputs compiled RISC-V assembly (`output.s`).

## Prerequisites
To build this backend, you must have a local LLVM build directory available (LLVM 23.0.0 or compatible).

## Building the Backend

1. Navigate to the backend directory and create a build folder:
   ```bash
   mkdir build && cd build
   ```
2. Configure CMake by pointing it to your LLVM installation:
   ```bash
   cmake -DLLVM_DIR=/path/to/your/llvm-project/build/lib/cmake/llvm ..
   ```
3. Compile the backend and driver:
   ```bash
   make -j4
   ```
This will produce the `rv32i-compiler` executable in `build/driver/`.

## Running and Verifying
To verify the compiler's output on the Spike simulator:

1. Write a C program (`test.c`) that interacts with Spike's `tohost` memory-mapped I/O port for exit status.
2. Emit LLVM IR from Clang (use `-O0` to preserve control flow):
   ```bash
   clang -S -emit-llvm -O0 --target=riscv32 -mabi=ilp32 test.c -o test.ll
   ```
3. Compile the IR to Assembly using our backend:
   ```bash
   ./build/driver/rv32i-compiler test.ll
   mv output.s test.s
   ```
4. Assemble and link into a bare-metal ELF using the GNU toolchain:
   ```bash
   riscv64-unknown-elf-gcc -march=rv32i -mabi=ilp32 -nostdlib -T link.ld start.s test.s -o test.elf
   ```
5. Simulate the executable using Spike:
   ```bash
   spike --isa=rv32i test.elf
   echo $?
   ```
If the backend generated correct code, the simulator will cleanly execute the instructions and return your expected exit code.
