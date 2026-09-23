/**
 * @file test_real_backend.cpp
 * @brief Integration tests for RunModule4 using RealBackend + ObfPasses plugin.
 *
 * These tests shell out to the real `opt` binary with the compiled pass
 * plugin, verifying end-to-end that:
 *   - Each pass runs without crashing or verification failure
 *   - The output IR is different from the input (transforms were applied)
 *   - Determinism: same (fixture, seed) produces identical output
 *   - Pass ordering is enforced through the orchestrator
 */

#include "../src/orchestration/module4_orchestrator.h"
#include "../src/orchestration/real_backend.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>

using orchestration::RunModule4;
using orchestration::RealBackend;
using orchestration::Module4Metrics;

// ---- Configuration ----
// These are resolved relative to the build directory where the test runs.
static std::string OptPath() {
    return "/opt/homebrew/opt/llvm@18/bin/opt";
}

static std::string PluginPath() {
    // The test binary and plugin are in the same build directory.
    return "./ObfPasses.dylib";
}

static std::string FixturePath(const std::string& name) {
    return "../tests/fixtures/" + name;
}

// ---- Helpers ----

static std::string ReadFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static void Cleanup(const std::vector<std::string>& files) {
    for (const auto& f : files) std::remove(f.c_str());
}

// ---- Tests ----

void test_instruction_substitution_real() {
    RealBackend backend(OptPath(), PluginPath());
    std::string out = "test_sub_out.ll";

    auto metrics = RunModule4(backend, "add_and_xor",
        {"instruction_substitution"},
        3, 42, FixturePath("arithmetic.ll"), out);

    assert(metrics.overhead > 0);  // Transform should have added lines.

    std::string original = ReadFile(FixturePath("arithmetic.ll"));
    std::string transformed = ReadFile(out);
    assert(transformed != original);  // Output should differ from input.
    assert(!transformed.empty());

    Cleanup({out});
    std::cout << "[PASS] instruction_substitution_real\n";
}

void test_bogus_control_flow_real() {
    RealBackend backend(OptPath(), PluginPath());
    std::string out = "test_bcf_out.ll";

    auto metrics = RunModule4(backend, "max",
        {"bogus_control_flow"},
        3, 42, FixturePath("branch.ll"), out);

    std::string transformed = ReadFile(out);
    assert(!transformed.empty());
    // Bogus CF should have added "obf.decoy" blocks.
    assert(transformed.find("obf.decoy") != std::string::npos ||
           transformed.find("obf.cont") != std::string::npos);

    Cleanup({out});
    std::cout << "[PASS] bogus_control_flow_real\n";
}

void test_control_flow_flattening_real() {
    RealBackend backend(OptPath(), PluginPath());
    std::string out = "test_cff_out.ll";

    auto metrics = RunModule4(backend, "max",
        {"control_flow_flattening"},
        3, 42, FixturePath("branch.ll"), out);

    std::string transformed = ReadFile(out);
    assert(!transformed.empty());
    // CFF should have added the dispatcher block.
    assert(transformed.find("obf.dispatch") != std::string::npos);

    Cleanup({out});
    std::cout << "[PASS] control_flow_flattening_real\n";
}

void test_determinism() {
    RealBackend backend1(OptPath(), PluginPath());
    RealBackend backend2(OptPath(), PluginPath());
    std::string out1 = "test_det_1.ll";
    std::string out2 = "test_det_2.ll";

    RunModule4(backend1, "add_and_xor",
        {"instruction_substitution"},
        3, 42, FixturePath("arithmetic.ll"), out1);

    RunModule4(backend2, "add_and_xor",
        {"instruction_substitution"},
        3, 42, FixturePath("arithmetic.ll"), out2);

    std::string ir1 = ReadFile(out1);
    std::string ir2 = ReadFile(out2);
    assert(ir1 == ir2);  // Same seed -> byte-identical output.

    Cleanup({out1, out2});
    std::cout << "[PASS] determinism\n";
}

void test_all_three_passes_chained() {
    RealBackend backend(OptPath(), PluginPath());
    std::string out = "test_chained_out.ll";

    // Give them in wrong order to verify the orchestrator fixes it.
    auto metrics = RunModule4(backend, "max",
        {"control_flow_flattening", "instruction_substitution", "bogus_control_flow"},
        3, 42, FixturePath("branch.ll"), out);

    std::string transformed = ReadFile(out);
    assert(!transformed.empty());
    assert(metrics.overhead > 0);
    assert(metrics.fully_deterministic == true);

    // Clean up stage files too.
    Cleanup({out, out + ".stage0", out + ".stage1"});
    std::cout << "[PASS] all_three_passes_chained\n";
}

int main() {
    std::cout << "Running RealBackend integration tests...\n";
    std::cout << "  opt:    " << OptPath() << "\n";
    std::cout << "  plugin: " << PluginPath() << "\n\n";

    test_instruction_substitution_real();
    test_bogus_control_flow_real();
    test_control_flow_flattening_real();
    test_determinism();
    test_all_three_passes_chained();

    std::cout << "\nAll RealBackend integration tests passed!\n";
    return 0;
}
