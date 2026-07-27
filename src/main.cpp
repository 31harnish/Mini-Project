#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>
#include <iostream>
#include "planner/build_controller.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_llvm_ir.ll>" << std::endl;
        return 1;
    }

    llvm::LLVMContext Context;
    llvm::SMDiagnostic Err;
    std::unique_ptr<llvm::Module> M = llvm::parseIRFile(argv[1], Err, Context);

    if (!M) {
        Err.print(argv[0], llvm::errs());
        return 1;
    }

    std::cout << "Successfully parsed LLVM IR: " << argv[1] << std::endl;
    planner::run_obfuscation_pipeline(*M);

    return 0;
}
