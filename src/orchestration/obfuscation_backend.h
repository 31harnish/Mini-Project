#ifndef OBFUSCATION_BACKEND_H
#define OBFUSCATION_BACKEND_H

#include <string>
#include <cstdint>

namespace orchestration {

/**
 * @brief Result of a single pass invocation against the backend.
 *
 * The orchestrator inspects verified_ok after every call and aborts
 * the pipeline on the first failure.  seed_respected is false when
 * the underlying API has no seed parameter for this particular pass,
 * so the caller can flag the run as non-deterministic.
 */
struct PassCallResult {
    bool        verified_ok       = false;
    int         instructions_added = 0;
    int         blocks_added       = 0;
    bool        seed_respected     = true;
    std::string error_message;
};

/**
 * @brief Abstraction boundary between the orchestrator and whatever
 *        the real local LLVM tool's API turns out to be.
 *
 * RunModule4 depends only on this interface, never on a concrete
 * backend, so the real implementation can be swapped in later without
 * touching resolver / params / orchestration logic.
 */
class IObfuscationBackend {
 public:
    virtual ~IObfuscationBackend() = default;

    /**
     * @param pass_name   One of: "instruction_substitution",
     *                    "bogus_control_flow", "control_flow_flattening", etc.
     * @param probability Transform probability (0.0–1.0), derived from intensity.
     * @param variant_pool Number of rewrite variants to choose from.
     * @param seed        RNG seed for deterministic output.
     * @param in_path     Path to input .ll file.
     * @param out_path    Path to write transformed .ll file.
     * @return PassCallResult with verification status and metrics.
     */
    virtual PassCallResult InvokePass(const std::string& pass_name,
                                       double probability,
                                       int variant_pool,
                                       uint64_t seed,
                                       const std::string& in_path,
                                       const std::string& out_path) = 0;
};

}  // namespace orchestration

#endif // OBFUSCATION_BACKEND_H
