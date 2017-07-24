
#include "llvm/IR/Module.h"
#include "llvm/ADT/SmallVector.h"

#include "DomTreePass.h"

using namespace giprofiler;

namespace giprofiler {

char DomTreePass::ID = 0;
std::string DomTreePass::passName = "DomTreePass";

}

bool
DomTreePass::immediatePostDominates(llvm::Instruction* def, llvm::Instruction* use) {

  if (def->getParent() == use->getParent()){
    return false;
  }


  return pdt->dominates(def->getParent(), use->getParent());
}

std::vector<llvm::Instruction*>
DomTreePass::immediatePostDominators(llvm::Function *fn, llvm::Instruction *def) {
  if (fn != f) {
    return {};
  }
  std::vector<llvm::Instruction*> dominators;

  for (auto& bb : *fn) {
    if (&bb == def->getParent()) {
      continue;
    }
    for (auto& i : bb) {
      if (pdt->dominates(&bb, def->getParent())) {
        dominators.push_back(&i);
      }
    }  
  }
  return dominators;
}

bool
DomTreePass::runOnFunction(llvm::Function& f) {
  this->f = &f;

  pdtwp = std::unique_ptr<llvm::PostDominatorTreeWrapperPass>(
    new llvm::PostDominatorTreeWrapperPass()
  );
  pdtwp->runOnFunction(f);
  llvm::PostDominatorTree& prepdt = pdtwp->getPostDomTree();
  pdt = &prepdt;

  return false;
}