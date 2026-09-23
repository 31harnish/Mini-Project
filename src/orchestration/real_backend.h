#ifndef REAL_BACKEND_H
#define REAL_BACKEND_H

#include "obfuscation_backend.h"
#include <string>

namespace orchestration {

/**
 * @brief Production backend that invokes passes via `opt -load-pass-plugin`.
 *
 * Shells out to the LLVM `opt` tool with the ObfPasses plugin loaded,
 * passing probability, seed, and variant pool as CLI flags.  Parses
 * opt's exit code for verification failure and reads the output file
 * size delta for basic metrics.
 *
 * The plugin_path must point to the compiled libObfPasses.dylib.
 * The opt_path must point to the LLVM 18 `opt` binary.
 */
class RealBackend : public IObfuscationBackend {
 public:
    /**
     * @param opt_path     Absolute path to the `opt` binary.
     * @param plugin_path  Absolute path to the libObfPasses.dylib plugin.
     */
    RealBackend(const std::string& opt_path, const std::string& plugin_path);

    PassCallResult InvokePass(const std::string& pass_name,
                               double probability,
                               int variant_pool,
                               uint64_t seed,
                               const std::string& in_path,
                               const std::string& out_path) override;

 private:
    std::string opt_path_;
    std::string plugin_path_;

    /** Map orchestrator pass names to opt -passes= names. */
    static std::string MapPassName(const std::string& pass_name);
};

}  // namespace orchestration

#endif // REAL_BACKEND_H
