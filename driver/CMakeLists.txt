add_executable(rv32i-compiler compiler.cpp)
target_link_libraries(rv32i-compiler PRIVATE LLVMRV32I)
llvm_map_components_to_libnames(llvm_libs Core IRReader Support CodeGen Target MC AsmPrinter SelectionDAG)
target_link_libraries(rv32i-compiler PRIVATE ${llvm_libs})
