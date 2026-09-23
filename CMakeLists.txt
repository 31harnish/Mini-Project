cmake_minimum_required(VERSION 3.16)
project(MiniProject LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
link_directories(${LLVM_LIBRARY_DIRS})

# Include directory for project interfaces
include_directories(include)

# Compile each module as its own object library
add_library(FeatureExtraction OBJECT src/analysis/feature_extraction.cpp)
add_library(ThreatAnalyzer OBJECT src/analysis/threat_analyzer.cpp)
add_library(ProtectionPlanner OBJECT src/planner/protection_planner.cpp src/planner/planner_utils.cpp)
add_library(BuildController OBJECT src/planner/build_controller.cpp)

# Module 4: Orchestration layer
add_library(Orchestration OBJECT
    src/orchestration/pass_order_resolver.cpp
    src/orchestration/intensity_params.cpp
    src/orchestration/module4_orchestrator.cpp
    src/orchestration/fake_backend.cpp
    src/orchestration/real_backend.cpp
)

# Module 4: LLVM pass plugin (loaded by opt at runtime)
add_library(ObfPasses MODULE src/passes/ExistingPasses.cpp)
target_compile_options(ObfPasses PRIVATE -fno-rtti)
# On macOS, MODULE libraries need undefined symbols to be allowed
# since they'll be resolved by the host process (opt).
set_target_properties(ObfPasses PROPERTIES
    PREFIX ""
    SUFFIX ".dylib"
    LINK_FLAGS "-undefined dynamic_lookup"
)

# Top-level executable / pass-plugin linking them all together
add_executable(obfuscation_framework 
    src/main.cpp
    $<TARGET_OBJECTS:FeatureExtraction>
    $<TARGET_OBJECTS:ThreatAnalyzer>
    $<TARGET_OBJECTS:ProtectionPlanner>
    $<TARGET_OBJECTS:BuildController>
    $<TARGET_OBJECTS:Orchestration>
)

# LLVM component linking
llvm_map_components_to_libnames(llvm_libs Core IRReader Support Analysis Passes)
target_link_libraries(obfuscation_framework ${llvm_libs})

# --- Test targets ---

# Test: PassOrderResolver
add_executable(test_pass_order
    tests/orchestration/test_pass_order_resolver.cpp
    $<TARGET_OBJECTS:Orchestration>
)

# Test: IntensityToParams
add_executable(test_intensity_params
    tests/orchestration/test_intensity_params.cpp
    $<TARGET_OBJECTS:Orchestration>
)

# Test: Module4 Orchestrator (needs FakeBackend)
add_executable(test_orchestration
    tests/orchestration/test_module4_orchestrator.cpp
    $<TARGET_OBJECTS:Orchestration>
)

# Test: RealBackend integration (needs ObfPasses.dylib built first)
add_executable(test_real_backend
    tests/orchestration/test_real_backend.cpp
    $<TARGET_OBJECTS:Orchestration>
)
add_dependencies(test_real_backend ObfPasses)

