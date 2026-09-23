/**
 * @file real_backend.cpp
 * @brief RealBackend implementation — invokes passes via `opt -load-pass-plugin`.
 *
 * For each InvokePass call, constructs and executes a shell command:
 *   opt -load-pass-plugin=<plugin> \
 *       -passes=<pass-name> \
 *       -obf-probability=<prob> -obf-seed=<seed> -obf-variants=<pool> \
 *       <in.ll> -S -o <out.ll>
 *
 * A non-zero exit code from opt is treated as a verification failure.
 * Basic metrics (instructions_added, blocks_added) are estimated from
 * the line-count delta between input and output files.
 */

#include "real_backend.h"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

namespace orchestration {

RealBackend::RealBackend(const std::string& opt_path,
                           const std::string& plugin_path)
    : opt_path_(opt_path), plugin_path_(plugin_path) {}

std::string RealBackend::MapPassName(const std::string& pass_name) {
    // The orchestrator uses underscored names (e.g. "instruction_substitution")
    // but the opt plugin registers hyphenated names (e.g. "instruction-substitution").
    if (pass_name == "instruction_substitution") return "instruction-substitution";
    if (pass_name == "bogus_control_flow")       return "bogus-control-flow";
    if (pass_name == "control_flow_flattening")  return "control-flow-flattening";
    // Fallback: replace underscores with hyphens.
    std::string mapped = pass_name;
    for (auto& c : mapped) if (c == '_') c = '-';
    return mapped;
}

static int CountLines(const std::string& path) {
    std::ifstream f(path);
    if (!f) return 0;
    int count = 0;
    std::string line;
    while (std::getline(f, line)) ++count;
    return count;
}

PassCallResult RealBackend::InvokePass(const std::string& pass_name,
                                        double probability,
                                        int variant_pool,
                                        uint64_t seed,
                                        const std::string& in_path,
                                        const std::string& out_path) {
    PassCallResult result{};

    std::string opt_pass_name = MapPassName(pass_name);

    // Count input lines for delta metrics.
    int lines_before = CountLines(in_path);

    // Build the opt command.
    std::ostringstream cmd;
    cmd << "\"" << opt_path_ << "\""
        << " -load-pass-plugin=\"" << plugin_path_ << "\""
        << " -passes=" << opt_pass_name
        << " -obf-probability=" << probability
        << " -obf-seed=" << seed
        << " -obf-variants=" << variant_pool
        << " \"" << in_path << "\""
        << " -S -o \"" << out_path << "\""
        << " 2>&1";

    std::cout << "  [RealBackend] " << cmd.str() << std::endl;

    // Execute opt.  Capture stderr/stdout for error reporting.
    FILE* pipe = popen(cmd.str().c_str(), "r");
    if (!pipe) {
        result.verified_ok = false;
        result.error_message = "RealBackend: failed to execute opt command";
        return result;
    }

    // Read all output (opt prints errors/warnings to stderr, redirected above).
    std::string output;
    char buf[256];
    while (fgets(buf, sizeof(buf), pipe)) {
        output += buf;
    }
    int exit_status = pclose(pipe);

    // On macOS, pclose returns the full wait status; extract the exit code.
    int exit_code = WEXITSTATUS(exit_status);

    if (exit_code != 0) {
        result.verified_ok = false;
        result.error_message = "RealBackend: opt exited with code " +
                                std::to_string(exit_code);
        if (!output.empty()) {
            result.error_message += "\nOutput: " + output;
        }
        return result;
    }

    // Check that the output file was actually created.
    std::ifstream out_check(out_path);
    if (!out_check.good()) {
        result.verified_ok = false;
        result.error_message = "RealBackend: opt succeeded but output file '" +
                                out_path + "' was not created";
        return result;
    }

    // Estimate metrics from line-count delta.
    int lines_after = CountLines(out_path);
    int line_delta = lines_after - lines_before;

    result.verified_ok       = true;
    result.instructions_added = (line_delta > 0) ? line_delta : 0;
    result.blocks_added       = result.instructions_added / 4;  // rough estimate
    result.seed_respected     = true;  // all three passes accept -obf-seed

    return result;
}

}  // namespace orchestration
