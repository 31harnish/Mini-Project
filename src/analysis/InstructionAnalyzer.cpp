#include "analyzers.h"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Instruction.h>
#include <set>

namespace analysis {

void InstructionAnalyzer::analyze(llvm::Function &F, FeatureVector &features) {
    unsigned totalInstructions = 0;
    unsigned loads = 0;
    unsigned stores = 0;
    unsigned allocas = 0;
    unsigned arithmeticOps = 0;
    unsigned logicOps = 0;
    unsigned compareOps = 0;

    std::set<unsigned> uniqueOpcodes;

    for (auto &BB : F) {
        for (auto &I : BB) {
            totalInstructions++;
            unsigned opcode = I.getOpcode();
            uniqueOpcodes.insert(opcode);

            if (llvm::isa<llvm::LoadInst>(&I)) {
                loads++;
            } else if (llvm::isa<llvm::StoreInst>(&I)) {
                stores++;
            } else if (llvm::isa<llvm::AllocaInst>(&I)) {
                allocas++;
            } else if (llvm::isa<llvm::CmpInst>(&I)) {
                compareOps++;
            } else if (llvm::isa<llvm::BinaryOperator>(&I)) {
                switch (opcode) {
                    case llvm::Instruction::Add:
                    case llvm::Instruction::FAdd:
                    case llvm::Instruction::Sub:
                    case llvm::Instruction::FSub:
                    case llvm::Instruction::Mul:
                    case llvm::Instruction::FMul:
                    case llvm::Instruction::UDiv:
                    case llvm::Instruction::SDiv:
                    case llvm::Instruction::FDiv:
                    case llvm::Instruction::URem:
                    case llvm::Instruction::SRem:
                    case llvm::Instruction::FRem:
                        arithmeticOps++;
                        break;
                    case llvm::Instruction::Shl:
                    case llvm::Instruction::LShr:
                    case llvm::Instruction::AShr:
                    case llvm::Instruction::And:
                    case llvm::Instruction::Or:
                    case llvm::Instruction::Xor:
                        logicOps++;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    features.instructions = totalInstructions;
    features.loads = loads;
    features.stores = stores;
    features.allocas = allocas;
    features.arithmetic_ops = arithmeticOps;
    features.logic_ops = logicOps;
    features.compare_ops = compareOps;

    // Derived Metrics
    if (totalInstructions > 0) {
        features.instruction_diversity = static_cast<double>(uniqueOpcodes.size()) / totalInstructions;
        features.memory_intensity = static_cast<double>(loads + stores) / totalInstructions;
    } else {
        features.instruction_diversity = 0.0;
        features.memory_intensity = 0.0;
    }
}

} // namespace analysis
