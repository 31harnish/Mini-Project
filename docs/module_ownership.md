# Module Ownership

| Module | File Path | Owner | Status | Depends On |
| :--- | :--- | :--- | :--- | :--- |
| 1. Feature Extraction | `src/analysis/feature_extraction.cpp` | GURURAJ S | stub | None |
| 2. Threat Analyzer | `src/analysis/threat_analyzer.cpp` | GURURAJ S | stub | Module 1 |
| 3. Protection Planner | `src/planner/protection_planner.cpp` | GOPAL R| stub | Module 1, Module 2 |
| 4. Existing Passes | `src/passes/existing/ollvm_passes.cpp` | HARNISHRAJ K | stub | Module 3 |
| 5. Novel Passes | `src/passes/novel/ai_resistant_passes.cpp` | HARNISHRAJ K | stub | Module 3 |
| 6. Diversification | `src/diversification/diversification_engine.cpp` | BADRINATH S | stub | Module 3 |
| 7. Evaluation Engine | `src/eval/evaluation_engine.cpp` | BADRINATH S | stub | Module 8 (orchestration output) |
| 8. Build Controller | `src/planner/build_controller.cpp` | GOPAL R | stub | Modules 1-7 |
