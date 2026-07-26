#ifndef AI_RESISTANT_PASSES_H
#define AI_RESISTANT_PASSES_H

#include "../../../include/interfaces.h"

namespace llvm {
    class Function;
}

namespace passes {
namespace novel {
    bool apply_ai_resistant_passes(llvm::Function& F, const ProtectionPlan& plan);
}
}

#endif // AI_RESISTANT_PASSES_H
