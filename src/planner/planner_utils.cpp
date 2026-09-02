#include "planner_utils.h"
#include <sstream>

namespace planner {

std::string dumpPlanToJson(const ProtectionPlan& plan) {
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"function\": \"" << plan.function_name << "\",\n";
    ss << "  \"intensity\": " << plan.intensity_level << ",\n";
    ss << "  \"rounds\": " << plan.max_transformation_rounds << ",\n";
    ss << "  \"target_accuracy\": " << plan.target_llm_reconstruction_accuracy << ",\n";
    ss << "  \"passes\": [\n";
    
    for (size_t i = 0; i < plan.selected_passes.size(); ++i) {
        ss << "    \"" << plan.selected_passes[i] << "\"";
        if (i < plan.selected_passes.size() - 1) {
            ss << ",\n";
        } else {
            ss << "\n";
        }
    }
    
    ss << "  ]\n";
    ss << "}\n";
    return ss.str();
}

}
