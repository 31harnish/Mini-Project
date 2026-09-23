/**
 * @file ExistingPasses.cpp
 * @brief LLVM-18 pass plugin implementing three obfuscation transforms.
 *
 * Exposes the following passes to `opt -passes=`:
 *   - instruction-substitution
 *   - bogus-control-flow
 *   - control-flow-flattening
 *
 * Each pass reads shared CLI options for probability, seed, and variant pool,
 * so RunModule4's RealBackend can control behavior via command-line flags.
 *
 * Build: cmake builds this as a MODULE library (libObfPasses.so/.dylib).
 * Run:   opt -load-pass-plugin=./libObfPasses.so \
 *            -passes=instruction-substitution \
 *            -obf-probability=0.6 -obf-seed=42 -obf-variants=2 \
 *            in.ll -S -o out.ll
 */

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/Local.h"
#include <map>
#include <random>
#include <vector>

using namespace llvm;

// ---- Shared CLI options (read by all three passes) ----
static cl::opt<double> Probability("obf-probability", cl::init(0.5));
static cl::opt<uint64_t> Seed("obf-seed", cl::init(0));
static cl::opt<int> VariantPool("obf-variants", cl::init(2));

namespace {

std::mt19937_64 MakeRng(Function &F, uint64_t BaseSeed) {
  // Combine global seed with function name so different functions in the
  // same module diverge, but the same (module, seed) pair is reproducible.
  std::hash<std::string> H;
  return std::mt19937_64(BaseSeed ^ H(F.getName().str()));
}

// ============================================================
// Pass 1: Instruction Substitution
// ============================================================
struct InstructionSubstitutionPass
    : public PassInfoMixin<InstructionSubstitutionPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    auto Rng = MakeRng(F, Seed);
    std::uniform_real_distribution<double> Chance(0.0, 1.0);
    std::uniform_int_distribution<int> Variant(0, VariantPool - 1);
    bool Changed = false;

    std::vector<Instruction *> Targets;
    for (auto &BB : F)
      for (auto &I : BB)
        if (I.getType()->isIntegerTy() && (I.getOpcode() == Instruction::Add ||
                                           I.getOpcode() == Instruction::Sub ||
                                           I.getOpcode() == Instruction::Xor))
          Targets.push_back(&I);

    for (auto *I : Targets) {
      if (Chance(Rng) > Probability)
        continue;

      IRBuilder<> B(I);
      Value *A = I->getOperand(0);
      Value *Bv = I->getOperand(1);
      Value *Repl = nullptr;
      int V = Variant(Rng);

      if (I->getOpcode() == Instruction::Add) {
        if (V == 0)
          // a + b == (a ^ b) + 2*(a & b)
          Repl = B.CreateAdd(B.CreateXor(A, Bv),
                             B.CreateMul(B.CreateAnd(A, Bv),
                                         ConstantInt::get(I->getType(), 2)));
        else
          // a + b == (a | b) + (a & b)
          Repl = B.CreateAdd(B.CreateOr(A, Bv), B.CreateAnd(A, Bv));
      } else if (I->getOpcode() == Instruction::Sub) {
        // a - b == a + (~b + 1)
        Repl = B.CreateAdd(
            A, B.CreateAdd(B.CreateNot(Bv), ConstantInt::get(I->getType(), 1)));
      } else if (I->getOpcode() == Instruction::Xor) {
        // a ^ b == (a | b) - (a & b)
        Repl = B.CreateSub(B.CreateOr(A, Bv), B.CreateAnd(A, Bv));
      }

      if (Repl) {
        I->replaceAllUsesWith(Repl);
        I->eraseFromParent();
        Changed = true;
      }
    }
    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
};

// ============================================================
// Pass 2: Bogus Control Flow
// ============================================================
struct BogusControlFlowPass : public PassInfoMixin<BogusControlFlowPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    auto Rng = MakeRng(F, Seed ^ 0x1234);
    std::uniform_real_distribution<double> Chance(0.0, 1.0);
    bool Changed = false;

    std::vector<BasicBlock *> Targets;
    for (auto &BB : F)
      if (&BB != &F.getEntryBlock() && !BB.empty() && !isa<PHINode>(BB.front()))
        Targets.push_back(&BB);

    for (auto *BB : Targets) {
      if (Chance(Rng) > Probability)
        continue;

      BasicBlock *Cont =
          BB->splitBasicBlock(BB->getFirstInsertionPt(), "obf.cont");
      BB->getTerminator()->eraseFromParent();

      IRBuilder<> B(BB);
      // Contextual opaque predicate derived from a live value rather than
      // a compile-time constant: (x*x) % 2 always equals (x % 2), so this
      // branch always resolves to true but can't be trivially constant-
      // folded by a naive optimizer.
      Value *X = nullptr;
      for (auto &Arg : F.args())
        if (Arg.getType()->isIntegerTy()) {
          X = &Arg;
          break;
        }
      if (!X)
        X = ConstantInt::get(Type::getInt32Ty(F.getContext()), 7);

      Value *X32 = B.CreateZExtOrTrunc(X, Type::getInt32Ty(F.getContext()));
      Value *Sq = B.CreateMul(X32, X32);
      Value *Lhs = B.CreateURem(Sq, ConstantInt::get(Sq->getType(), 2));
      Value *Rhs = B.CreateURem(X32, ConstantInt::get(X32->getType(), 2));
      Value *Cond = B.CreateICmpEQ(Lhs, Rhs); // always true

      BasicBlock *Decoy = BasicBlock::Create(F.getContext(), "obf.decoy", &F);
      IRBuilder<> DB(Decoy);
      DB.CreateUnreachable();

      B.CreateCondBr(Cond, Cont, Decoy);
      Changed = true;
    }
    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
};

// ============================================================
// Pass 3: Control Flow Flattening
// ============================================================
struct ControlFlowFlatteningPass
    : public PassInfoMixin<ControlFlowFlatteningPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    if (F.size() < 2)
      return PreservedAnalyses::all();
    // Skip functions with exception-handling to avoid corrupting landingpads.
    for (auto &BB : F)
      if (BB.isLandingPad())
        return PreservedAnalyses::all();

    auto Rng = MakeRng(F, Seed ^ 0xABCD);

    // Demote PHIs first so the CFG can be safely torn apart.
    std::vector<PHINode *> Phis;
    for (auto &BB : F)
      for (auto &I : BB)
        if (auto *P = dyn_cast<PHINode>(&I))
          Phis.push_back(P);
    for (auto *P : Phis)
      DemotePHIToStack(P);

    LLVMContext &Ctx = F.getContext();
    IntegerType *I32 = Type::getInt32Ty(Ctx);

    // Split the original entry block: the first half becomes a clean
    // "prologue" that only contains allocas + the initial state store +
    // the branch to the dispatcher. The second half (old entry code)
    // becomes a regular body block that the dispatcher targets.
    BasicBlock &OrigEntry = F.getEntryBlock();
    Instruction *SplitPt = nullptr;
    for (auto &I : OrigEntry) {
      if (!isa<AllocaInst>(&I)) {
        SplitPt = &I;
        break;
      }
    }
    if (!SplitPt)
      SplitPt = OrigEntry.getTerminator();

    BasicBlock *OldEntryBody =
        OrigEntry.splitBasicBlock(SplitPt, "obf.entry.body");

    // Collect all body blocks (everything except the prologue/entry).
    std::vector<BasicBlock *> Body;
    for (auto &BB : F)
      if (&BB != &OrigEntry)
        Body.push_back(&BB);
    if (Body.empty())
      return PreservedAnalyses::all();

    // Assign randomized state IDs.
    std::vector<int> StateIds(Body.size());
    for (size_t i = 0; i < Body.size(); ++i)
      StateIds[i] = i;
    std::shuffle(StateIds.begin(), StateIds.end(), Rng);

    // Map BasicBlock* -> state ID for rewriting terminators.
    std::map<BasicBlock *, int> BlockToState;
    for (size_t i = 0; i < Body.size(); ++i)
      BlockToState[Body[i]] = StateIds[i];

    int InitialState = BlockToState[OldEntryBody];

    // ResolveState must exist BEFORE the rewrite loop below uses it, and
    // must handle &OrigEntry as a special case: after splitting, OrigEntry
    // has no real logic left, so any edge that used to target it (e.g. a
    // loop back to the top of the function) should re-enter at
    // OldEntryBody's state instead.
    auto ResolveState = [&](BasicBlock *Succ) -> int {
      if (Succ == &OrigEntry)
        return InitialState;
      auto It = BlockToState.find(Succ);
      return It != BlockToState.end() ? It->second : InitialState;
    };

    // Create the dispatcher block.
    BasicBlock *Dispatch = BasicBlock::Create(Ctx, "obf.dispatch", &F);

    // Prologue: state alloca, store initial state, branch to dispatcher.
    OrigEntry.getTerminator()->eraseFromParent();
    IRBuilder<> EB(&OrigEntry);
    AllocaInst *StateVar = EB.CreateAlloca(I32, nullptr, "obf.state");
    EB.CreateStore(ConstantInt::get(I32, InitialState), StateVar);
    EB.CreateBr(Dispatch);

    // Dispatcher: load state, apply XOR indirection, switch.
    IRBuilder<> DB(Dispatch);
    Value *LoadedState = DB.CreateLoad(I32, StateVar, "obf.state.load");
    Value *Transformed =
        DB.CreateXor(LoadedState, ConstantInt::get(I32, 0x5A5A));

    SwitchInst *Switch = DB.CreateSwitch(Transformed, Dispatch, Body.size());
    for (size_t i = 0; i < Body.size(); ++i) {
      int Xored = StateIds[i] ^ 0x5A5A;
      Switch->addCase(ConstantInt::get(I32, Xored), Body[i]);
    }

    // Rewrite each body block's terminator to go through the dispatcher,
    // using ResolveState (not a raw map lookup) so edges back to OrigEntry
    // are handled correctly instead of being silently dropped.
    for (auto *BB : Body) {
      Instruction *Term = BB->getTerminator();
      if (!Term)
        continue;

      if (auto *BI = dyn_cast<BranchInst>(Term)) {
        if (BI->isUnconditional()) {
          BasicBlock *Succ = BI->getSuccessor(0);
          int NextState = ResolveState(Succ);
          IRBuilder<> B(BB);
          Term->eraseFromParent();
          B.CreateStore(ConstantInt::get(I32, NextState), StateVar);
          B.CreateBr(Dispatch);
        } else {
          BasicBlock *TrueSucc = BI->getSuccessor(0);
          BasicBlock *FalseSucc = BI->getSuccessor(1);
          Value *Cond = BI->getCondition();

          int TrueState = ResolveState(TrueSucc);
          int FalseState = ResolveState(FalseSucc);

          IRBuilder<> B(BB);
          Term->eraseFromParent();
          Value *NextState =
              B.CreateSelect(Cond, ConstantInt::get(I32, TrueState),
                             ConstantInt::get(I32, FalseState));
          B.CreateStore(NextState, StateVar);
          B.CreateBr(Dispatch);
        }
      }
      // ReturnInst terminators stay as-is — they exit the function.
    }

    // Flattening invalidates normal dominance for ANY cross-block value,
    // not just PHIs. Collect targets first, then demote in a separate
    // pass — mutating instructions while iterating over them would
    // invalidate the iterator.
    std::vector<Instruction *> ToDemote;
    for (auto *BB : Body) {
      for (auto &I : *BB) {
        bool CrossBlockUse = false;
        for (User *U : I.users()) {
          if (auto *UI = dyn_cast<Instruction>(U)) {
            if (UI->getParent() != BB) {
              CrossBlockUse = true;
              break;
            }
          }
        }
        if (CrossBlockUse)
          ToDemote.push_back(&I);
      }
    }
    for (auto *I : ToDemote)
      DemoteRegToStack(*I, /*VolatileLoads=*/false);

    return PreservedAnalyses::none();
  }
};

} // namespace

// ============================================================
// Plugin registration — exposes passes to `opt -passes=`
// ============================================================
PassPluginLibraryInfo getObfPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "ObfPasses", "v1", [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "instruction-substitution") {
                    FPM.addPass(InstructionSubstitutionPass());
                    return true;
                  }
                  if (Name == "bogus-control-flow") {
                    FPM.addPass(BogusControlFlowPass());
                    return true;
                  }
                  if (Name == "control-flow-flattening") {
                    FPM.addPass(ControlFlowFlatteningPass());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getObfPassPluginInfo();
}