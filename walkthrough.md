# Module 4 — Orchestration Layer Walkthrough

## What was built

The Module 4 orchestration layer — a backend-agnostic pipeline that resolves pass ordering, maps intensity to parameters, and invokes transforms through a swappable interface.

## Files created

### Source (`src/orchestration/`)

| File | Purpose |
|------|---------|
| [`obfuscation_backend.h`](file:///Users/guru/Documents/Mini-Project/src/orchestration/obfuscation_backend.h) | `IObfuscationBackend` interface + `PassCallResult` struct |
| [`fake_backend.h`](file:///Users/guru/Documents/Mini-Project/src/orchestration/fake_backend.h) | `FakeBackend` test double with failure injection |
| [`fake_backend.cpp`](file:///Users/guru/Documents/Mini-Project/src/orchestration/fake_backend.cpp) | FakeBackend implementation — copies IR, returns synthetic metrics |
| [`pass_order_resolver.h`](file:///Users/guru/Documents/Mini-Project/src/orchestration/pass_order_resolver.h) | `ResolvePassOrder()` with full ordering contract documentation |
| [`pass_order_resolver.cpp`](file:///Users/guru/Documents/Mini-Project/src/orchestration/pass_order_resolver.cpp) | Stable-sort implementation using priority values |
| [`intensity_params.h`](file:///Users/guru/Documents/Mini-Project/src/orchestration/intensity_params.h) | `IntensityToParams()` + `PassParams` struct |
| [`intensity_params.cpp`](file:///Users/guru/Documents/Mini-Project/src/orchestration/intensity_params.cpp) | Table-driven mapping for all 7 passes x 5 intensity levels |
| [`module4_orchestrator.h`](file:///Users/guru/Documents/Mini-Project/src/orchestration/module4_orchestrator.h) | `RunModule4()` signature + `Module4Metrics` |
| [`module4_orchestrator.cpp`](file:///Users/guru/Documents/Mini-Project/src/orchestration/module4_orchestrator.cpp) | Core orchestration loop |

### Tests (`tests/orchestration/`)

| File | Tests |
|------|-------|
| [`test_pass_order_resolver.cpp`](file:///Users/guru/Documents/Mini-Project/tests/orchestration/test_pass_order_resolver.cpp) | 13 tests: permutations, edge cases, stability, priority values |
| [`test_intensity_params.cpp`](file:///Users/guru/Documents/Mini-Project/tests/orchestration/test_intensity_params.cpp) | 11 tests: monotonic scaling for all 7 passes, ranges, error cases |
| [`test_module4_orchestrator.cpp`](file:///Users/guru/Documents/Mini-Project/tests/orchestration/test_module4_orchestrator.cpp) | 8 tests: happy path, abort-on-failure, seed gap flagging, order enforcement, metrics scaling |

### Modified

| File | Change |
|------|--------|
| [`interfaces.h`](file:///Users/guru/Documents/Mini-Project/include/interfaces.h) | Added `random_seed` to `ProtectionPlan`, added `EvaluationMetrics` struct |
| [`CMakeLists.txt`](file:///Users/guru/Documents/Mini-Project/CMakeLists.txt) | Added `Orchestration` library + 3 test executables |

## Test results

All **31 tests pass** across 3 test suites:

```
Running PassOrderResolver tests...
[PASS] all_three_already_ordered
[PASS] all_three_reversed
[PASS] all_three_mixed
[PASS] sub_before_flat_1
[PASS] sub_before_flat_2
[PASS] single_bogus / single_substitution / single_flattening
[PASS] empty_list
[PASS] unknown_passes_sorted_last
[PASS] stable_sort_equal_priority
[PASS] full_planner_pass_set
[PASS] priority_values
All PassOrderResolver tests passed!

Running IntensityToParams tests...
[PASS] monotonic_* (7 passes)
[PASS] valid_ranges
[PASS] specific_values
[PASS] unknown_pass_throws
[PASS] intensity_out_of_range_throws
All IntensityToParams tests passed!

Running Module4 Orchestrator tests...
[PASS] happy_path_three_passes
[PASS] pass_order_enforced
[PASS] failure_aborts_pipeline
[PASS] seed_gap_flagging
[PASS] all_seeds_supported
[PASS] single_pass
[PASS] empty_passes
[PASS] metrics_scale_with_intensity
All Module4 Orchestrator tests passed!
```

The main `obfuscation_framework` binary continues to build and run correctly on `simple.ll`.

## Pass Ordering Contract

> **Instruction substitution (priority 1) -> Bogus control flow (priority 2) -> Control flow flattening (priority 3)**
>
> Substitution operates on individual instructions within existing basic blocks. Flattening replaces the original block layout with a switch-based dispatcher. If substitution ran after flattening, it would operate on dispatcher bookkeeping instructions rather than user code — producing weaker results and risking verifier failures. The resolver enforces this regardless of the order the planner provides.

## Real Backend Integration

The orchestrator is now fully wired up to a real LLVM 18 pass plugin!

### Files added
| File | Purpose |
|------|---------|
| [`ExistingPasses.cpp`](file:///Users/guru/Documents/Mini-Project/src/passes/ExistingPasses.cpp) | The actual LLVM 18 passes (Substitution, Bogus CF, Flattening) built as `ObfPasses.dylib`. |
| [`real_backend.h`](file:///Users/guru/Documents/Mini-Project/src/orchestration/real_backend.h) | Production backend that shells out to `opt -load-pass-plugin`. |
| [`real_backend.cpp`](file:///Users/guru/Documents/Mini-Project/src/orchestration/real_backend.cpp) | Implements the opt invocation and parses metrics from line-count deltas. |
| [`test_real_backend.cpp`](file:///Users/guru/Documents/Mini-Project/tests/orchestration/test_real_backend.cpp) | End-to-end integration tests verifying the actual passes via the orchestrator. |

### Bug Fixes
The `ControlFlowFlatteningPass` initially failed LLVM's `VerifierPass` because the dispatcher block made the entry block have a predecessor, and the CFG restructuring broke dominance rules for allocas.

This was fixed by:
1. Splitting the `entry` block: leaving the `alloca` instructions in a clean prologue.
2. Moving the rest of the original entry block into the flattened dispatcher body.
3. Explicitly collecting and demoting cross-block register uses to stack (`DemoteRegToStack`) prior to rewriting the CFG.

### Test results

All **36 tests pass** across 4 test suites, including the end-to-end integration tests:

```
Running RealBackend integration tests...
  opt:    /opt/homebrew/opt/llvm@18/bin/opt
  plugin: ./ObfPasses.dylib

[PASS] instruction_substitution_real
[PASS] bogus_control_flow_real
[PASS] control_flow_flattening_real
[PASS] determinism
[PASS] all_three_passes_chained

All RealBackend integration tests passed!
```



# Module 4 End-to-End Pipeline Validation Report

## Fixture: [`high_risk.ll`](file:///Users/guru/Documents/Mini-Project/tests/fixtures/high_risk.ll)

| Metric | Value |
|---|---|
| Function name | `encrypt_block` |
| Basic Block Count | 12 |
| Cyclomatic Complexity | 5 |
| Instruction Diversity | 9 |
| Sensitive | Yes (keyword: "encrypt") |
| Risk Score | **0.665** |
| Sensitivity Category | cryptography |
| **Intensity Level** | **3** |

---

## Module 3 ProtectionPlan Output

```json
{
  "function": "encrypt_block",
  "intensity": 3,
  "rounds": 2,
  "target_accuracy": 0.55,
  "passes": [
    "semantic_divergence",
    "bogus_control_flow",
    "adaptive_cfg_diversification"
  ]
}
```

**3 passes selected** at intensity 3 ✅ (Includes both Module 4 and pending Module 5 passes).

---

## Orchestration Layer & Module 4 Passes Applied

The `build_controller.cpp` now successfully partitions passes between Module 4 (implemented) and Module 5 (pending). 

### Module 5 (Skipped)
- `[Module 4] Skipping pass 'semantic_divergence' — pending Module 5 implementation`
- `[Module 4] Skipping pass 'adaptive_cfg_diversification' — pending Module 5 implementation`

### Module 4 (Executed)
| # | Pass | Priority | Prob | Pool | Status |
|---|---|---|---|---|---|
| 1 | `bogus_control_flow` | 2 (block-level) | 0.5 | 2 | ✅ |

**Aggregate metrics**: 28 instructions added, 7 blocks added, fully deterministic ✅

---

## Verification

| Check | Result |
|---|---|
| `opt -passes=verify` (exit code) | **0** ✅ |
| `llvm::verifyModule()` in `main.cpp` | **Passed** ✅ |
| Re-parse output IR | **Succeeded** ✅ |

---

## Bug Found & Fixed

### 1. Protection Planner Pass Selection Mutilation Reverted
> [!NOTE]
> A previous session incorrectly removed all Module 5 passes from the `protection_planner.cpp` pass allocation logic to prevent pipeline crashes. This has been **reverted**. The planner now correctly assigns passes like `semantic_divergence`, `decoy_idiom`, and `string_obfuscation` according to the original architecture design.

### 2. Sensitivity Category Typo
> [!NOTE]
> The `"encryption"` sensitivity category check on line 80 didn't match anything — `threat_analyzer.cpp` produces `"cryptography"`, never `"encryption"`. **This remains fixed** to `"cryptography"`.

### 3. Build Controller Skip Logic Added
> [!NOTE]
> The orchestrator in `build_controller.cpp` was updated to gracefully handle the fact that Module 5 is not yet built. It now filters the `selected_passes` array into `M4_PASSES` and `M5_PASSES`. It invokes `RunModule4` with only the `M4_PASSES` subset and prints an explicit skip log for any `M5_PASSES` rather than crashing the pipeline on missing opt plugins.
