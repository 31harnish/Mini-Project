#ifndef FAKE_BACKEND_H
#define FAKE_BACKEND_H

#include "obfuscation_backend.h"
#include <map>
#include <set>
#include <string>

namespace orchestration {

/**
 * @brief Test double for IObfuscationBackend.
 *
 * No real transformation happens.  Copies the input IR file verbatim
 * to the output path and returns synthetic-but-plausible metrics, so
 * RunModule4's control flow (ordering, error propagation, metrics
 * aggregation, abort-on-failure) can be exercised without the real
 * tool being available.
 *
 * Configurable failure injection:
 *   - FailOn(pass_name)           -> that pass reports verification failure
 *   - SetSeedUnsupported(pass_name) -> that pass reports seed_respected=false
 */
class FakeBackend : public IObfuscationBackend {
 public:
    PassCallResult InvokePass(const std::string& pass_name,
                               double probability,
                               int variant_pool,
                               uint64_t seed,
                               const std::string& in_path,
                               const std::string& out_path) override;

    /** Make the named pass report a verification failure. */
    void FailOn(const std::string& pass_name) {
        forced_failures_.insert(pass_name);
    }

    /** Mark the named pass as not supporting seed propagation. */
    void SetSeedUnsupported(const std::string& pass_name) {
        seed_unsupported_.insert(pass_name);
    }

    /** How many times InvokePass was called for this pass name. */
    int CallCountFor(const std::string& pass_name) const;

    /** Order in which passes were actually invoked (for order assertions). */
    const std::vector<std::string>& InvocationOrder() const {
        return invocation_order_;
    }

 private:
    std::set<std::string>        forced_failures_;
    std::set<std::string>        seed_unsupported_;
    std::map<std::string, int>   call_counts_;
    std::vector<std::string>     invocation_order_;
};

}  // namespace orchestration

#endif // FAKE_BACKEND_H
