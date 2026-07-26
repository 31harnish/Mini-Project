/**
 * @file ollvm_passes.cpp
 * @brief Module 4 — Existing LLVM Obfuscation Passes (PoC Implementation)
 */

#include "ollvm_passes.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <vector>
#include <map>
#include <iostream>

using namespace llvm;

namespace passes {
namespace existing {

// 1. Instruction Substitution: a = b + c -> a = (b ^ c) + 2 * (b & c)
static void applyInstructionSubstitution(Function& F) {
    std::vector<Instruction*> toReplace;
    for (auto& BB : F) {
        for (auto& I : BB) {
            if (I.getOpcode() == Instruction::Add && I.getType()->isIntegerTy()) {
                toReplace.push_back(&I);
            }
        }
    }

    for (auto* I : toReplace) {
        IRBuilder<> builder(I);
        Value* op1 = I->getOperand(0);
        Value* op2 = I->getOperand(1);
        
        Value* xorVal = builder.CreateXor(op1, op2);
        Value* andVal = builder.CreateAnd(op1, op2);
        Value* shlVal = builder.CreateShl(andVal, ConstantInt::get(I->getType(), 1));
        Value* newVal = builder.CreateAdd(xorVal, shlVal);
        
        I->replaceAllUsesWith(newVal);
        I->eraseFromParent();
    }
}

// 2. Bogus Control Flow: Inject opaque predicates
static void applyBogusControlFlow(Function& F) {
    std::vector<BasicBlock*> blocks;
    for (auto& BB : F) {
        blocks.push_back(&BB);
    }
    
    LLVMContext& Ctx = F.getContext();
    for (auto* BB : blocks) {
        if (BB->getTerminator() == nullptr) continue;
        
        Instruction* term = BB->getTerminator();
        BasicBlock* origTail = BB->splitBasicBlock(term, "bcf_orig_tail");
        BB->getTerminator()->eraseFromParent();
        
        IRBuilder<> builder(BB);
        // Opaque predicate: 1 == 1
        Value* cmp = builder.CreateICmpEQ(builder.getInt32(1), builder.getInt32(1));
        
        BasicBlock* junkBB = BasicBlock::Create(Ctx, "bcf_junk", &F);
        IRBuilder<> junkBuilder(junkBB);
        junkBuilder.CreateAdd(builder.getInt32(1), builder.getInt32(2));
        junkBuilder.CreateBr(origTail);
        
        builder.CreateCondBr(cmp, origTail, junkBB);
    }
}

// 3. Control Flow Flattening (PoC)
static void applyControlFlowFlattening(Function& F) {
    if (F.empty()) return;
    
    std::vector<BasicBlock*> origBlocks;
    for (auto& BB : F) {
        if (!BB.isEHPad()) {
            origBlocks.push_back(&BB);
        }
    }
    if (origBlocks.size() <= 1) return;
    
    LLVMContext& Ctx = F.getContext();
    BasicBlock* entryBB = &F.getEntryBlock();
    
    // Split the entry block to avoid flattening alloca instructions which must remain in entry
    Instruction* splitPoint = entryBB->getTerminator();
    BasicBlock* newEntry = entryBB->splitBasicBlock(splitPoint, "cff_entry");
    origBlocks[0] = newEntry; // Replace entry block with its split remainder

    BasicBlock* loopEntry = BasicBlock::Create(Ctx, "cff_loop_entry", &F);
    BasicBlock* loopEnd = BasicBlock::Create(Ctx, "cff_loop_end", &F);
    BasicBlock* switchBlock = BasicBlock::Create(Ctx, "cff_switch", &F);

    // Create state variable in original entry block
    entryBB->getTerminator()->eraseFromParent();
    IRBuilder<> entryBuilder(entryBB);
    AllocaInst* stateVar = entryBuilder.CreateAlloca(Type::getInt32Ty(Ctx), 0, "cff_state");
    entryBuilder.CreateStore(entryBuilder.getInt32(1), stateVar); // Start with state 1
    entryBuilder.CreateBr(loopEntry);

    IRBuilder<> loopEntryBuilder(loopEntry);
    LoadInst* stateLoad = loopEntryBuilder.CreateLoad(Type::getInt32Ty(Ctx), stateVar, "cff_state_load");
    loopEntryBuilder.CreateBr(switchBlock);

    IRBuilder<> switchBuilder(switchBlock);
    SwitchInst* switchInst = switchBuilder.CreateSwitch(stateLoad, loopEnd, origBlocks.size());
    
    IRBuilder<>(loopEnd).CreateBr(loopEntry);

    std::map<BasicBlock*, int> blockToState;
    int stateCounter = 1;
    for (auto* BB : origBlocks) {
        blockToState[BB] = stateCounter++;
    }

    for (auto* BB : origBlocks) {
        switchInst->addCase(switchBuilder.getInt32(blockToState[BB]), BB);
        
        // Rewrite branches to state updates
        Instruction* term = BB->getTerminator();
        if (BranchInst* br = dyn_cast<BranchInst>(term)) {
            if (br->isUnconditional()) {
                BasicBlock* succ = br->getSuccessor(0);
                if (blockToState.count(succ)) {
                    IRBuilder<> builder(br);
                    builder.CreateStore(builder.getInt32(blockToState[succ]), stateVar);
                    builder.CreateBr(loopEnd);
                    br->eraseFromParent();
                }
            } else if (br->isConditional()) {
                BasicBlock* trueSucc = br->getSuccessor(0);
                BasicBlock* falseSucc = br->getSuccessor(1);
                if (blockToState.count(trueSucc) && blockToState.count(falseSucc)) {
                    IRBuilder<> builder(br);
                    Value* sel = builder.CreateSelect(
                        br->getCondition(),
                        builder.getInt32(blockToState[trueSucc]),
                        builder.getInt32(blockToState[falseSucc])
                    );
                    builder.CreateStore(sel, stateVar);
                    builder.CreateBr(loopEnd);
                    br->eraseFromParent();
                }
            }
        }
    }
}

// 4. String Encryption (Mock implementation operating on local variables for PoC)
static void applyStringEncryption(Function& F) {
    // True string encryption requires module-level pass to mutate GlobalVariables.
    // As a PoC for a function pass, we will just scan for constant arrays and log.
    // The actual transformation is left as an advanced exercise.
    for (auto& BB : F) {
        for (auto& I : BB) {
            // Placeholder: find store of constant string or something similar, replace it...
        }
    }
}

bool apply_ollvm_passes(llvm::Function& F, const ProtectionPlan& plan) {
    bool transformed = false;
    for (const auto& pass_name : plan.selected_passes) {
        if (pass_name == "InstructionSubstitution") {
            applyInstructionSubstitution(F);
            transformed = true;
        } else if (pass_name == "BogusControlFlow") {
            applyBogusControlFlow(F);
            transformed = true;
        } else if (pass_name == "ControlFlowFlattening") {
            applyControlFlowFlattening(F);
            transformed = true;
        } else if (pass_name == "StringEncryption") {
            applyStringEncryption(F);
            transformed = true;
        }
    }
    return transformed;
}

} // namespace existing
} // namespace passes
