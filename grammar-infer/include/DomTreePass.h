
#ifndef DOM_TREE_PASS_H
#define DOM_TREE_PASS_H

#include "llvm/IR/Module.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/PostDominators.h"

namespace giprofiler {

struct DomTreePass : public llvm::FunctionPass {

  static char ID;
  static std::string passName;
  std::unique_ptr<llvm::PostDominatorTreeWrapperPass> pdtwp;
  llvm::PostDominatorTree *pdt;
  llvm::Function* f;

  llvm::DenseMap<llvm::BasicBlock*, unsigned> basicBlockIDMap;

  DomTreePass() : llvm::FunctionPass(ID) { }

  virtual const char*
  getPassName() const override {
    return DomTreePass::passName.c_str();
  }

  std::vector<llvm::Instruction*> immediatePostDominators(
    llvm::Function *fn, llvm::Instruction *def
  );
  bool immediatePostDominates(llvm::Instruction *def, llvm::Instruction *use);
  bool runOnFunction(llvm::Function& f) override;

};

}

#endif