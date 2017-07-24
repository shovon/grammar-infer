

#ifndef PROFILING_INSTRUMENTATION_PASS_H
#define PROFILING_INSTRUMENTATION_PASS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"


namespace cgprofiler {


struct ProfilingInstrumentationPass : public llvm::ModulePass {

  static char ID;
  llvm::DenseMap<llvm::Instruction*, uint64_t> instructionIds;
  llvm::DenseMap<llvm::Instruction*, uint64_t> callsiteIds;
  llvm::DenseMap<llvm::Function*, uint64_t> targetIds;

  ProfilingInstrumentationPass()
    : llvm::ModulePass(ID)
      {}

  bool runOnModule(llvm::Module& m) override;
  

















  void handleInstruction(llvm::CallSite cs, llvm::Value* counter, llvm::Value* csUpdater);
  //void handleCalledFunction(llvm::Module&, llvm::Function& f);
  void indexCallsite(llvm::CallSite cs, uint64_t index);
  void createCallsiteTable(llvm::Module& m, std::vector<llvm::CallSite> callsites,
                           std::vector<llvm::Function*> targets);
};


}


#endif

