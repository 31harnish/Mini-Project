/**
 * @file test_module4_orchestrator.cpp
 * @brief Integration tests for RunModule4 using FakeBackend.
 *
 * Verifies:
 *   - Happy path: all passes invoked in order, metrics aggregated
 *   - Failure abort: pipeline stops on first verification failure
 *   - Seed gap flagging: non-deterministic passes are flagged
 *   - Single pass: works correctly with one pass
 *   - Empty passes: returns zero metrics with no backend calls
 *   - Pass order enforcement: substitution always before flattening
 */

#include "../src/orchestration/module4_orchestrator.h"
#include "../src/orchestration/fake_backend.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstdio>

using orchestration::RunModule4;
using orchestration::FakeBackend;
using orchestration::Module4Metrics;

// ---- Helpers ----

static const char* kTestInputFile  = "test_input.ll";
static const char* kTestOutputFile = "test_output.ll";

void CreateDummyIR(const char* path) {
    std::ofstream f(path);
    f << "; ModuleID = 'test'\n"
      << "define i32 @test_func(i32 %a, i32 %b) {\n"
      << "  %sum = add i32 %a, %b\n"
      << "  ret i32 %sum\n"
      << "}\n";
}

void Cleanup() {
    std::remove(kTestInputFile);
    std::remove(kTestOutputFile);
    // Clean up any stage files.
    for (int i = 0; i < 10; ++i) {
        std::string stage = std::string(kTestOutputFile) + ".stage" + std::to_string(i);
        std::remove(stage.c_str());
    }
}

// ---- Tests ----

void test_happy_path_three_passes() {
    CreateDummyIR(kTestInputFile);
    FakeBackend backend;

    auto metrics = RunModule4(backend, "test_func",
        {"instruction_substitution", "bogus_control_flow", "control_flow_flattening"},
        3, 42, kTestInputFile, kTestOutputFile);

    // All three passes should have been called exactly once.
    assert(backend.CallCountFor("instruction_substitution") == 1);
    assert(backend.CallCountFor("bogus_control_flow") == 1);
    assert(backend.CallCountFor("control_flow_flattening") == 1);

    // Metrics should be non-zero (FakeBackend scales by probability).
    assert(metrics.overhead > 0.0);
    assert(metrics.size_bloat > 0.0);
    assert(metrics.fully_deterministic == true);

    // Output file should exist.
    std::ifstream out(kTestOutputFile);
    assert(out.good());

    Cleanup();
    std::cout << "[PASS] happy_path_three_passes\n";
}

void test_pass_order_enforced() {
    CreateDummyIR(kTestInputFile);
    FakeBackend backend;

    // Give passes in wrong order — orchestrator should fix it.
    RunModule4(backend, "test_func",
        {"control_flow_flattening", "instruction_substitution", "bogus_control_flow"},
        3, 42, kTestInputFile, kTestOutputFile);

    // Check the actual invocation order recorded by FakeBackend.
    const auto& order = backend.InvocationOrder();
    assert(order.size() == 3);
    assert(order[0] == "instruction_substitution");
    assert(order[1] == "bogus_control_flow");
    assert(order[2] == "control_flow_flattening");

    Cleanup();
    std::cout << "[PASS] pass_order_enforced\n";
}

void test_failure_aborts_pipeline() {
    CreateDummyIR(kTestInputFile);
    FakeBackend backend;
    backend.FailOn("bogus_control_flow");

    bool threw = false;
    try {
        RunModule4(backend, "test_func",
            {"instruction_substitution", "bogus_control_flow", "control_flow_flattening"},
            3, 42, kTestInputFile, kTestOutputFile);
    } catch (const std::runtime_error& e) {
        threw = true;
        // Error message should mention the failing pass.
        std::string msg = e.what();
        assert(msg.find("bogus_control_flow") != std::string::npos);
    }
    assert(threw);

    // Substitution should have run once before the failure.
    assert(backend.CallCountFor("instruction_substitution") == 1);
    // Bogus was called but failed.
    assert(backend.CallCountFor("bogus_control_flow") == 1);
    // Flattening should never have been reached.
    assert(backend.CallCountFor("control_flow_flattening") == 0);

    Cleanup();
    std::cout << "[PASS] failure_aborts_pipeline\n";
}

void test_seed_gap_flagging() {
    CreateDummyIR(kTestInputFile);
    FakeBackend backend;
    backend.SetSeedUnsupported("control_flow_flattening");

    auto metrics = RunModule4(backend, "test_func",
        {"instruction_substitution", "control_flow_flattening"},
        2, 42, kTestInputFile, kTestOutputFile);

    // Pipeline should complete, but flag as non-deterministic.
    assert(metrics.fully_deterministic == false);

    Cleanup();
    std::cout << "[PASS] seed_gap_flagging\n";
}

void test_all_seeds_supported() {
    CreateDummyIR(kTestInputFile);
    FakeBackend backend;  // No SetSeedUnsupported calls.

    auto metrics = RunModule4(backend, "test_func",
        {"instruction_substitution", "bogus_control_flow"},
        2, 42, kTestInputFile, kTestOutputFile);

    assert(metrics.fully_deterministic == true);

    Cleanup();
    std::cout << "[PASS] all_seeds_supported\n";
}

void test_single_pass() {
    CreateDummyIR(kTestInputFile);
    FakeBackend backend;

    auto metrics = RunModule4(backend, "test_func",
        {"bogus_control_flow"},
        4, 99, kTestInputFile, kTestOutputFile);

    assert(backend.CallCountFor("bogus_control_flow") == 1);
    assert(metrics.overhead > 0.0);

    Cleanup();
    std::cout << "[PASS] single_pass\n";
}

void test_empty_passes() {
    CreateDummyIR(kTestInputFile);
    FakeBackend backend;

    auto metrics = RunModule4(backend, "test_func",
        {},
        3, 42, kTestInputFile, kTestOutputFile);

    // No passes = no calls, zero metrics.
    assert(metrics.overhead == 0.0);
    assert(metrics.size_bloat == 0.0);
    assert(metrics.fully_deterministic == true);

    Cleanup();
    std::cout << "[PASS] empty_passes\n";
}

void test_metrics_scale_with_intensity() {
    CreateDummyIR(kTestInputFile);

    // Run at intensity 1.
    FakeBackend backend_low;
    auto m_low = RunModule4(backend_low, "test_func",
        {"instruction_substitution"},
        1, 42, kTestInputFile, kTestOutputFile);

    // Run at intensity 5.
    CreateDummyIR(kTestInputFile);
    FakeBackend backend_high;
    auto m_high = RunModule4(backend_high, "test_func",
        {"instruction_substitution"},
        5, 42, kTestInputFile, kTestOutputFile);

    // Higher intensity should produce higher overhead (FakeBackend
    // scales metrics by probability, which scales with intensity).
    assert(m_high.overhead >= m_low.overhead);

    Cleanup();
    std::cout << "[PASS] metrics_scale_with_intensity\n";
}

int main() {
    std::cout << "Running Module4 Orchestrator tests...\n";
    test_happy_path_three_passes();
    test_pass_order_enforced();
    test_failure_aborts_pipeline();
    test_seed_gap_flagging();
    test_all_seeds_supported();
    test_single_pass();
    test_empty_passes();
    test_metrics_scale_with_intensity();
    std::cout << "All Module4 Orchestrator tests passed!\n";
    return 0;
}
