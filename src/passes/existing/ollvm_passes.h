#ifndef OLLVM_PASSES_H
#define OLLVM_PASSES_H

#include "../../include/interfaces.h"

namespace llvm {
    class Function;
}

namespace passes {
namespace existing {
    bool apply_ollvm_passes(llvm::Function& F, const ProtectionPlan& plan);
}
}

#endif // OLLVM_PASSES_H
