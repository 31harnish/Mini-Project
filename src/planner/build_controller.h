#ifndef BUILD_CONTROLLER_H
#define BUILD_CONTROLLER_H

#include "../../include/interfaces.h"
#include <string>

namespace llvm {
    class Module;
}

namespace planner {
    /**
     * @brief Run the full obfuscation pipeline (Modules 1-3 analysis + planning).
     *
     * @param M  The parsed LLVM Module for analysis/feature extraction.
     */
    void run_obfuscation_pipeline(llvm::Module& M);

    /**
     * @brief Run the full pipeline including Module 4 (orchestration).
     *
     * Runs Modules 1-3, then for each function's ProtectionPlan calls
     * RunModule4 via RealBackend to apply passes on the IR file.
     *
     * @param M             The parsed LLVM Module for analysis.
     * @param ir_in_path    Path to the input .ll file on disk.
     * @param ir_out_path   Path to write the obfuscated .ll output.
     * @param opt_path      Path to the LLVM `opt` binary.
     * @param plugin_path   Path to ObfPasses.dylib.
     */
    void run_full_pipeline(llvm::Module& M,
                           const std::string& ir_in_path,
                           const std::string& ir_out_path,
                           const std::string& opt_path,
                           const std::string& plugin_path);
}

#endif // BUILD_CONTROLLER_H
