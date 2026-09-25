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
