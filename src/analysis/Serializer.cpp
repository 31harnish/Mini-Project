#include "analyzers.h"
#include <llvm/Support/JSON.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>

namespace analysis {

void FeatureSerializer::serialize(const std::vector<FeatureVector> &featuresList, const std::string &outputPath) {
    llvm::json::Array jsonList;

    for (const auto &fv : featuresList) {
        llvm::json::Object jsonFv;
        jsonFv["function"] = fv.function_name;

        // Structural
        jsonFv["basic_blocks"] = fv.basic_block_count;
        jsonFv["instructions"] = fv.instructions;
        jsonFv["cyclomatic"] = fv.cyclomatic_complexity;
        jsonFv["cfg_density"] = fv.cfg_density;

        // Control Flow
        jsonFv["branches"] = fv.branch_count;
        jsonFv["switches"] = fv.switch_count;
        jsonFv["loops"] = fv.loop_count;
        jsonFv["max_loop_depth"] = fv.max_loop_depth;

        // Calls
        jsonFv["calls"] = fv.call_count;
        jsonFv["external_calls"] = fv.external_calls;
        jsonFv["recursive"] = fv.recursive;

        // Memory
        jsonFv["loads"] = fv.loads;
        jsonFv["stores"] = fv.stores;
        jsonFv["allocas"] = fv.allocas;

        // Instruction Mix
        jsonFv["arithmetic_ops"] = fv.arithmetic_ops;
        jsonFv["logic_ops"] = fv.logic_ops;
        jsonFv["compare_ops"] = fv.compare_ops;

        // Security
        jsonFv["crypto"] = fv.crypto_api;
        jsonFv["authentication"] = fv.auth_api;
        jsonFv["fileio"] = fv.file_io;
        jsonFv["networkio"] = fv.network_io;

        jsonFv["secret_strings"] = fv.secret_strings;
        jsonFv["integer_constants"] = fv.integer_constants;

        // Derived
        jsonFv["instruction_diversity"] = fv.instruction_diversity;
        jsonFv["memory_intensity"] = fv.memory_intensity;

        jsonList.push_back(std::move(jsonFv));
    }

    std::error_code ec;
    llvm::raw_fd_ostream os(outputPath, ec, llvm::sys::fs::OF_None);
    if (ec) {
        llvm::errs() << "Error: Could not open output file for serialization: " << ec.message() << "\n";
        return;
    }

    // Format output with nice spacing
    os << llvm::formatv("{0:2}", llvm::json::Value(std::move(jsonList)));
}

} // namespace analysis
