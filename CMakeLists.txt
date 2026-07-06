cmake_minimum_required(VERSION 3.16.0)
project(RV32IBackend)

# ------------------------------------------------------------------------------
# LLVM Configuration
# ------------------------------------------------------------------------------
# Default to your custom built LLVM project directory if LLVM_DIR isn't set via CLI
if(NOT DEFINED LLVM_DIR)
    set(LLVM_DIR "/home/shikhar/Desktop/toy-mlir/llvm-project/build/lib/cmake/llvm" CACHE PATH "Path to LLVM cmake configs")
endif()

if(NOT DEFINED LLVM_TABLEGEN_EXE)
    set(LLVM_TABLEGEN_EXE "/home/shikhar/Desktop/toy-mlir/llvm-project/build/bin/llvm-tblgen" CACHE PATH "Path to llvm-tblgen")
endif()

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include LLVM CMake utilities for TableGen and AddLLVM macros
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)
include(TableGen)

# Set standard LLVM compiler flags, includes, and links
add_definitions(${LLVM_DEFINITIONS})
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
link_directories(${LLVM_LIBRARY_DIRS})
# LLVM >= 16 requires C++17. If you are using LLVM 18/23, this is strictly required.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure RTTI and Exceptions match LLVM's build configuration
if(NOT LLVM_ENABLE_RTTI)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()
if(NOT LLVM_ENABLE_EH)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
endif()

# ------------------------------------------------------------------------------
# 1. TableGen Files (Placeholder)
# ------------------------------------------------------------------------------
set(LLVM_TARGET_DEFINITIONS RV32I.td)
tablegen(LLVM RV32IGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM RV32IGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM RV32IGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM RV32IGenCallingConv.inc -gen-callingconv)
tablegen(LLVM RV32IGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM RV32IGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM RV32IGenMCCodeEmitter.inc -gen-emitter)
add_public_tablegen_target(RV32ITableGen)

# ------------------------------------------------------------------------------
# 2. C++ Source Files
# ------------------------------------------------------------------------------
set(RV32I_SOURCES
    RV32ITargetInfo.cpp
    RV32ITargetMachine.cpp
    RV32ISubtarget.cpp
    RV32ITargetObjectFile.cpp
    RV32IInstrInfo.cpp
    RV32IRegisterInfo.cpp
    RV32ISelLowering.cpp
    RV32ISelDAGtoDAG.cpp
    RV32IFrameLowering.cpp
    RV32IAsmPrinter.cpp
    MCTargetDesc/RV32IMCTargetDesc.cpp
    MCTargetDesc/RV32IInstPrinter.cpp
    MCTargetDesc/RV32IMCAsmInfo.cpp
    MCTargetDesc/RV32ITargetStreamer.cpp
    MCTargetDesc/RV32IMCCodeEmitter.cpp
    MCTargetDesc/RV32IABIInfo.cpp
)

# ------------------------------------------------------------------------------
# 3. Build Target
# ------------------------------------------------------------------------------
if(RV32I_SOURCES)
    add_llvm_library(LLVMRV32I
        ${RV32I_SOURCES}
        DEPENDS
        RV32ITableGen
        LINK_COMPONENTS
        Core
        Support
        CodeGen
        Target
        MC
    )

endif()

add_subdirectory(driver)
