#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include <vector>
#include <string>
#include <iostream>
#include "ProfilingInstrumentationPass.h"

#include "DomTreePass.h"

using namespace std;
using namespace cgprofiler;
using namespace llvm;

namespace cgprofiler {

  char ProfilingInstrumentationPass::ID = 0;

} // namespace cgprofiler

struct statement {
  uint64_t line;
  llvm::Instruction* instruction;
  string type;
  uint64_t id;
  uint64_t postDom = 0;
}; 

static llvm::Constant*
createConstantString(llvm::Module& m, llvm::StringRef str) {
  auto& context = m.getContext();

  auto* name    = llvm::ConstantDataArray::getString(context, str, true);
  auto* int8Ty  = llvm::Type::getInt8Ty(context);
  auto* arrayTy = llvm::ArrayType::get(int8Ty, str.size() + 1);
  auto* asStr   = new llvm::GlobalVariable(
    m, arrayTy, true, llvm::GlobalValue::PrivateLinkage, name
  );

  auto* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  llvm::Value* indices[] = {zero, zero};
  return llvm::ConstantExpr::getInBoundsGetElementPtr(arrayTy, asStr, indices);
}

void handleCalledFunction(
  llvm::Module& m,
  statement s,
  DenseMap<Instruction*, Instruction*> fMap,
  DenseMap<Instruction*,
  uint64_t> ids
);

llvm::Instruction*
find(llvm::Value* val, DenseMap<Instruction*, Instruction*> fMap) {
    if (!isa<User>(val))  //Base case
    {
      return NULL;
    }

    auto* value = dyn_cast<User>(val);  //Need it in User form to use operand functions
    
    if (auto* AI = dyn_cast<CallInst>(value)) {  //Is call, possibly a read
      auto* c = AI->getCalledFunction();
      if (c->getName().equals("fgetc")) { //Is fgetc    
        return AI;
      }
    }
    
      //Check each operand for traces of fgetc
    for (unsigned j = 0; j < value->getNumOperands(); j++)
    {

      Instruction* v = find(value->getOperand(j), fMap);
      if (v != NULL)
      {
        return v;
      }
    }
    if (auto* AI = dyn_cast<Instruction>(value))
    {
      if (fMap[AI])
      {
        return fMap[AI];
      }
    }
    return NULL;

  }

bool
ProfilingInstrumentationPass::runOnModule(llvm::Module& m) {

  std::vector<statement> statements;
  llvm::DenseMap<Instruction*, statement*> sMap;
  llvm::DenseMap<Instruction*, Instruction*> fMap;
  int counter = 0;

  //Go through each instruction
  for (auto& f : m) {
    for (auto& bb : f) {
      for (auto& i : bb) {
        statement s;	
        sMap[&i] = &s;  //Store statement to map
        s.instruction = &i;
        
        if (DILocation *Loc = i.getDebugLoc()) {
          s.line = Loc->getLine();
        }

        if (auto* AI = dyn_cast<CallInst>(&i)) {
          //Is call, possibly a read
          auto* c = AI->getCalledFunction();

          if (c->getName().equals("fgetc")) {
            //Is fgetc     
            s.type = "GetChar";
          } else if(c->getName().equals("ungetc")) {
            s.type = "UngetChar";
          } else if(!c->isDeclaration()) {
            s.type = "MethodCall";
            for (auto& arg : AI->arg_operands()) {
              auto* inst = dyn_cast<Instruction>(arg);
              if (inst) {  
                find(inst, fMap);
              }
            }

          } else {
            s.type = "Other";
          }

        } else if (isa<CmpInst>(&i)) {	//Is a predicate
          s.type = "Predicate";
          find(&i, fMap);
        } else if(isa<StoreInst>(&i)) {
          s.type = "Other";

          Instruction* fi = find(i.getOperand(0), fMap);
          if (fi) {
            if (auto* AI = dyn_cast<Instruction>(i.getOperand(1))) {
              fMap[AI] = fi;  //Labelling 
            }
          }
          else {
            fMap[AI] = NULL;  //Unlabelling
          }
        } else if(isa<LoadInst>(&i)) {
          s.type = "Other";
        } else {      //Other
          s.type = "Other";
        }

        s.id = counter;
        instructionIds[&i] = counter;

        counter++;
        
        statements.push_back(s);
      }
    }
  }

  for(auto& s : statements) {
    if(s.type == "Predicate") {
      giprofiler::DomTreePass dtp;
      Function* function = s.instruction->getFunction();
      dtp.runOnFunction(*function);

      for(auto& bb : *s.instruction->getFunction()) {
        for(auto& i : bb) {
          if(dtp.immediatePostDominates( &i ,&*s.instruction )) {
            s.postDom = instructionIds[&i];
            goto label;
          }
        }
      }
    }
    label:
    continue;
  }
  for(auto s : statements) {
    handleCalledFunction(m, s, fMap, instructionIds);
  }
  return true;
}


void
handleCalledFunction(
  llvm::Module& m,
  statement s,
  DenseMap<Instruction*, Instruction*> fMap,
  DenseMap<Instruction*, uint64_t> ids
) {
  auto& context = m.getContext();

  IRBuilder<> builder(&*s.instruction);
  std::string str;
  std::string sid = to_string(s.id);
  std::string post = to_string(s.postDom);
  std::string line = to_string(s.line);
  str = str + sid + "|" + s.type  + "|" + post + "|" + line + "|";

  if(!s.type.compare("Predicate") || !s.type.compare("MethodCall")) {
    Instruction* f = find(s.instruction, fMap);
    if (f != NULL) {
      str+= to_string(ids[f]) + "|";
    }
  }
  StringRef sref(str);
  Constant* val = createConstantString(m, sref);

  auto* voidTy = Type::getVoidTy(context);
  auto* stringTy = Type::getInt8PtrTy(context);
  auto* helperTy = FunctionType::get(voidTy, stringTy, false);

  auto* called = m.getOrInsertFunction(
    "G1Pr0_called", helperTy);
  builder.CreateCall(called, val);
}