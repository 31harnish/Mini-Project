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
add_library(FeatureExtraction OBJECT 
    src/analysis/feature_extraction.cpp
    src/analysis/CFGAnalyzer.cpp
    src/analysis/ComplexityAnalyzer.cpp
    src/analysis/InstructionAnalyzer.cpp
    src/analysis/SecurityAnalyzer.cpp
    src/analysis/ConstantAnalyzer.cpp
    src/analysis/CallGraphAnalyzer.cpp
    src/analysis/Serializer.cpp
)
add_library(ThreatAnalyzer OBJECT src/analysis/threat_analyzer.cpp)
add_library(ProtectionPlanner OBJECT src/planner/protection_planner.cpp)
add_library(OLLVMPasses OBJECT src/passes/existing/ollvm_passes.cpp)
add_library(AIResistantPasses OBJECT src/passes/novel/ai_resistant_passes.cpp)
add_library(DiversificationEngine OBJECT src/diversification/diversification_engine.cpp)
add_library(EvaluationEngine OBJECT src/eval/evaluation_engine.cpp)
add_library(BuildController OBJECT src/planner/build_controller.cpp)

# Top-level executable / pass-plugin linking them all together
add_executable(obfuscation_framework 
    src/main.cpp
    $<TARGET_OBJECTS:FeatureExtraction>
    $<TARGET_OBJECTS:ThreatAnalyzer>
    $<TARGET_OBJECTS:ProtectionPlanner>
    $<TARGET_OBJECTS:OLLVMPasses>
    $<TARGET_OBJECTS:AIResistantPasses>
    $<TARGET_OBJECTS:DiversificationEngine>
    $<TARGET_OBJECTS:EvaluationEngine>
    $<TARGET_OBJECTS:BuildController>
)

# LLVM component linking
llvm_map_components_to_libnames(llvm_libs Core IRReader Support Analysis Passes)
target_link_libraries(obfuscation_framework ${llvm_libs})
