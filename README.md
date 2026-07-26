# Adaptive Multi-Pass LLVM Obfuscation Framework with Decoy Injection and Control-Flow Diversification

An LLVM IR pass pipeline that analyzes each function's complexity and sensitivity, adaptively decides which obfuscation passes to apply (existing OLLVM-style passes plus new AI-resistant passes we're contributing), applies them with per-build randomization so no two compiled binaries look alike, then evaluates protection strength against both static reverse-engineering tools and LLM-assisted decompilation.

## Pipeline Flow

1. **Feature Extraction Engine**: Extracts structural and syntactical features from LLVM IR functions.
2. **Security & Threat Analyzer**: Scores function risk and categorizes sensitivity.
3. **Protection Planner**: Rule-based logic mapping risk to obfuscation plans.
4. **Existing OLLVM Passes**: Core control-flow flattening, bogus control flow, etc.
5. **AI-Resistant Diversification Passes**: Decoy idiom injection, semantic divergence, symbol alteration.
6. **Diversification Engine**: Randomizes execution order and config based on seeds.
7. **Evaluation Engine**: Measures runtime/size overhead and resistance against RE.
8. **Build Controller**: Orchestrates the above modules into a cohesive pipeline.

## Build Instructions

1. Ensure LLVM (version 14 or higher) is installed.
2. Run standard CMake build:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

## Module Map

| Module | File Path | Description |
| :--- | :--- | :--- |
| 1 | `src/analysis/feature_extraction.cpp` | Extracts CFG density, complexity, API calls. |
| 2 | `src/analysis/threat_analyzer.cpp` | Analyzes threats and outputs risk scores. |
| 3 | `src/planner/protection_planner.cpp` | Plans the set of passes and their intensity. |
| 4 | `src/passes/existing/ollvm_passes.cpp` | Implements traditional OLLVM passes. |
| 5 | `src/passes/novel/ai_resistant_passes.cpp` | Implements AI-resistant passes (decoys, etc.). |
| 6 | `src/diversification/diversification_engine.cpp`| Orchestrates randomization of passes. |
| 7 | `src/eval/evaluation_engine.cpp` | Measures execution metrics and RE resistance. |
| 8 | `src/planner/build_controller.cpp` | Pipeline controller integrating everything. |

## Running the Pipeline

To run the pipeline on a test program:
```bash
./obfuscation_framework <path_to_llvm_ir.ll>
```