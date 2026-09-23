/**
 * @file fake_backend.cpp
 * @brief FakeBackend implementation — test double for IObfuscationBackend.
 */

#include "fake_backend.h"
#include <fstream>

namespace orchestration {

PassCallResult FakeBackend::InvokePass(const std::string& pass_name,
                                        double probability,
                                        int variant_pool,
                                        uint64_t seed,
                                        const std::string& in_path,
                                        const std::string& out_path) {
    call_counts_[pass_name]++;
    invocation_order_.push_back(pass_name);

    PassCallResult result{};

    // Forced failure injection for testing abort-on-failure.
    if (forced_failures_.count(pass_name)) {
        result.verified_ok = false;
        result.error_message =
            "FakeBackend: forced verification failure for '" + pass_name + "'";
        return result;
    }

    // Copy input -> output verbatim (stand-in for "transformed" IR).
    std::ifstream src(in_path, std::ios::binary);
    if (!src) {
        result.verified_ok = false;
        result.error_message =
            "FakeBackend: could not open input '" + in_path + "'";
        return result;
    }
    std::ofstream dst(out_path, std::ios::binary);
    dst << src.rdbuf();

    // Synthetic metrics, scaled by probability so tests can assert
    // "higher intensity -> more instructions added" invariants.
    result.verified_ok       = true;
    result.instructions_added = static_cast<int>(probability * 10);
    result.blocks_added       = static_cast<int>(probability * 3);
    result.seed_respected     = (seed_unsupported_.count(pass_name) == 0);

    return result;
}

int FakeBackend::CallCountFor(const std::string& pass_name) const {
    auto it = call_counts_.find(pass_name);
    return it == call_counts_.end() ? 0 : it->second;
}

}  // namespace orchestration
