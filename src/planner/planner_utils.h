#ifndef PLANNER_UTILS_H
#define PLANNER_UTILS_H

#include "../../include/interfaces.h"
#include <string>

namespace planner {

    /**
     * @brief Dumps a ProtectionPlan to a JSON formatted string.
     * 
     * @param plan The ProtectionPlan to serialize.
     * @return std::string JSON representation of the plan.
     */
    std::string dumpPlanToJson(const ProtectionPlan& plan);

}

#endif // PLANNER_UTILS_H
