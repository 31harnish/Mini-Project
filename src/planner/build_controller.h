#ifndef BUILD_CONTROLLER_H
#define BUILD_CONTROLLER_H

#include "../../include/interfaces.h"

namespace llvm {
    class Module;
}

namespace planner {
    void run_obfuscation_pipeline(llvm::Module& M);
}

#endif // BUILD_CONTROLLER_H
