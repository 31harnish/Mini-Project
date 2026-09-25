#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Verifier.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "planner/build_controller.h"

/**
 * Locate the ObfPasses.dylib plugin relative to the executable.
 * Tries ./ObfPasses.dylib first (build dir), falls back to same-dir as argv[0].
 */
static std::string find_plugin(const char* argv0) {
    // Try current working directory first (typical when run from build/).
    if (std::ifstream("ObfPasses.dylib").good())
        return "ObfPasses.dylib";

    // Try directory containing the executable.
    std::string dir(argv0);
    auto pos = dir.find_last_of('/');
    if (pos != std::string::npos) {
        std::string candidate = dir.substr(0, pos + 1) + "ObfPasses.dylib";
        if (std::ifstream(candidate).good())
            return candidate;
    }

    // Default — let the runtime report a clear error.
    return "ObfPasses.dylib";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_llvm_ir.ll> [opt_path]" << std::endl;
        return 1;
    }

    std::string ir_input = argv[1];
    std::string opt_path = (argc >= 3) ? argv[2] : "/opt/homebrew/opt/llvm@18/bin/opt";
    std::string plugin_path = find_plugin(argv[0]);
    std::string ir_output = "obfuscated.ll";

    llvm::LLVMContext Context;
    llvm::SMDiagnostic Err;
    std::unique_ptr<llvm::Module> M = llvm::parseIRFile(ir_input, Err, Context);

    if (!M) {
        Err.print(argv[0], llvm::errs());
        return 1;
    }

    std::cout << "Successfully parsed LLVM IR: " << ir_input << std::endl;

    // Run the full pipeline: Modules 1-3 (analysis/planning) + Module 4 (orchestration).
    planner::run_full_pipeline(*M, ir_input, ir_output, opt_path, plugin_path);

    // Verify the output IR is well-formed by re-parsing it.
    llvm::SMDiagnostic VerifyErr;
    std::unique_ptr<llvm::Module> Verified =
        llvm::parseIRFile(ir_output, VerifyErr, Context);
    if (!Verified) {
        std::cerr << "ERROR: Output IR failed to parse:" << std::endl;
        VerifyErr.print(argv[0], llvm::errs());
        return 1;
    }

    if (llvm::verifyModule(*Verified, &llvm::errs())) {
        std::cerr << "ERROR: Output IR failed verification." << std::endl;
        return 1;
    }

    std::cout << "Output IR verified OK: " << ir_output << std::endl;
    return 0;
}

