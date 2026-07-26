/**
 * @file ai_resistant_passes.cpp
 * @brief Module 5 — AI-Resistant Diversification Passes (PoC Implementation)
 */

#include "ai_resistant_passes.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <vector>

using namespace llvm;

namespace passes {
namespace novel {

// 1. Decoy Idiom Injection
static void applyDecoyIdiomInjection(Function& F) {
    if (F.empty()) return;
    LLVMContext& Ctx = F.getContext();
    
    BasicBlock* entryBB = &F.getEntryBlock();
    Instruction* term = entryBB->getTerminator();
    if (!term) return;

    // Create a branch to a decoy block that is never taken
    BasicBlock* origTail = entryBB->splitBasicBlock(term, "decoy_tail");
    entryBB->getTerminator()->eraseFromParent();

    IRBuilder<> builder(entryBB);
    // Opaque predicate: 0 != 0 (Always false)
    Value* cmp = builder.CreateICmpNE(builder.getInt32(0), builder.getInt32(0));
    
    BasicBlock* decoyBB = BasicBlock::Create(Ctx, "decoy_crypto_logic", &F);
    IRBuilder<> decoyBuilder(decoyBB);
    // Inject fake crypto idiom (e.g. CRC32 magic polynomial)
    Value* magic = decoyBuilder.getInt32(0x04C11DB7);
    Value* fakeVar = decoyBuilder.CreateXor(builder.getInt32(0xFFFFFFFF), magic);
    decoyBuilder.CreateBr(origTail); // jump to tail

    builder.CreateCondBr(cmp, decoyBB, origTail);
}

// 2. Semantic Divergence
static void applySemanticDivergence(Function& F) {
    std::vector<BasicBlock*> blocks;
    for (auto& BB : F) {
        blocks.push_back(&BB);
    }
    
    LLVMContext& Ctx = F.getContext();
    for (auto* BB : blocks) {
        if (BB->getTerminator() == nullptr) continue;
        
        IRBuilder<> builder(BB->getTerminator());
        // Insert some dead computation that looks plausible
        Value* x = builder.getInt32(100);
        Value* y = builder.getInt32(200);
        builder.CreateSub(y, x, "plausible_divergence"); // Never used
    }
}

// 3. Symbol / Metadata Alteration
static void applySymbolAlteration(Function& F) {
    // Rename basic blocks and instructions to misleading names
    int counter = 0;
    for (auto& BB : F) {
        if (!BB.getName().empty()) {
            BB.setName("decryption_step_" + std::to_string(counter++));
        }
        for (auto& I : BB) {
            if (!I.getName().empty()) {
                I.setName("key_material_" + std::to_string(counter++));
            }
        }
    }
}

bool apply_ai_resistant_passes(llvm::Function& F, const ProtectionPlan& plan) {
    bool transformed = false;
    for (const auto& pass_name : plan.selected_passes) {
        if (pass_name == "DecoyIdiomInjection") {
            applyDecoyIdiomInjection(F);
            transformed = true;
        } else if (pass_name == "SemanticDivergence") {
            applySemanticDivergence(F);
            transformed = true;
        } else if (pass_name == "SymbolAlteration") {
            applySymbolAlteration(F);
            transformed = true;
        }
    }
    return transformed;
}

} // namespace novel
} // namespace passes
