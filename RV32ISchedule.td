//===-- RV32ISchedule.td - RV32I Scheduling Definitions ----*- tablegen -*-===//
//
// LLVM includes a powerful Instruction Scheduler that reorders instructions 
// to maximize performance on superscalar or out-of-order CPUs.
//
// Since we are building a simple backend, we just provide a basic, 
// in-order generic model.
//
//===----------------------------------------------------------------------===//

/// GenericRV32Model:
/// Defines a simple in-order processor.
def GenericRV32Model : SchedMachineModel {
  let MicroOpBufferSize = 0; // Explicitly set to 0 for strict in-order execution.
  let IssueWidth = 1;        // Can only issue 1 micro-op per cycle.
  let LoadLatency = 2;       // Assumes memory loads take 2 cycles.
  let MispredictPenalty = 3; // Branch misprediction penalty of 3 cycles.
  let CompleteModel = 0;     // Not all instructions have scheduling data.
}
