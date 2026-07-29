#include "analyzers.h"
#include <llvm/Support/JSON.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Instructions.h>
#include <cstdlib>

namespace analysis {

SecurityAnalyzer::SecurityAnalyzer() {
    loadDatabase();
}

void SecurityAnalyzer::loadDatabase() {
    if (Loaded) return;

    std::string dbPath = "";
    if (const char* envPath = std::getenv("SECURITY_DB_PATH")) {
        dbPath = envPath;
    } else {
        dbPath = "SecurityDatabase.json";
    }

    // Try reading file
    auto bufferOrErr = llvm::MemoryBuffer::getFile(dbPath);
    if (!bufferOrErr) {
        // Fallback to absolute path in the workspace
        dbPath = "/Users/guru/Downloads/Mini-Project/SecurityDatabase.json";
        bufferOrErr = llvm::MemoryBuffer::getFile(dbPath);
    }

    if (!bufferOrErr) {
        // Fallback hardcoded defaults if file couldn't be loaded
        llvm::errs() << "Warning: Could not open SecurityDatabase.json. Using fallback embedded security database.\n";
        Database["crypto"] = {"AES_encrypt", "EVP_EncryptUpdate", "SHA256", "RSA_sign", "validate_key", "aes_like_cipher_block", "encrypt_data", "decrypt_data"};
        Database["authentication"] = {"strcmp", "bcrypt", "login", "authenticate_user", "pam_authenticate"};
        Database["filesystem"] = {"fopen", "CreateFile", "read", "write", "open", "close"};
        Database["network"] = {"socket", "connect", "send", "recv", "bind", "listen"};
        Loaded = true;
        return;
    }

    auto parseResult = llvm::json::parse(bufferOrErr.get()->getBuffer());
    if (!parseResult) {
        llvm::errs() << "Warning: Failed to parse SecurityDatabase.json. Using fallback embedded security database.\n";
        Database["crypto"] = {"AES_encrypt", "EVP_EncryptUpdate", "SHA256", "RSA_sign", "validate_key", "aes_like_cipher_block", "encrypt_data", "decrypt_data"};
        Database["authentication"] = {"strcmp", "bcrypt", "login", "authenticate_user", "pam_authenticate"};
        Database["filesystem"] = {"fopen", "CreateFile", "read", "write", "open", "close"};
        Database["network"] = {"socket", "connect", "send", "recv", "bind", "listen"};
        Loaded = true;
        return;
    }

    auto *obj = parseResult->getAsObject();
    if (!obj) {
        Loaded = true;
        return;
    }

    auto extractCategory = [&](const std::string& categoryKey) {
        if (auto *arr = obj->getArray(categoryKey)) {
            for (auto &val : *arr) {
                if (auto str = val.getAsString()) {
                    Database[categoryKey].push_back(str->str());
                }
            }
        }
    };

    extractCategory("crypto");
    extractCategory("authentication");
    extractCategory("filesystem");
    extractCategory("network");

    Loaded = true;
}

void SecurityAnalyzer::analyze(llvm::Function &F, FeatureVector &features) {
    features.crypto_api = false;
    features.auth_api = false;
    features.file_io = false;
    features.network_io = false;

    // Helper lambda to check if a function name matches entry list
    auto matchesCategory = [&](const std::string& name, const std::string& category) -> bool {
        if (Database.find(category) == Database.end()) return false;
        for (const auto& entry : Database[category]) {
            if (name.find(entry) != std::string::npos) {
                return true;
            }
        }
        return false;
    };

    for (auto &BB : F) {
        for (auto &I : BB) {
            if (auto *call = llvm::dyn_cast<llvm::CallBase>(&I)) {
                if (auto *callee = call->getCalledFunction()) {
                    std::string calleeName = callee->getName().str();
                    
                    if (matchesCategory(calleeName, "crypto")) {
                        features.crypto_api = true;
                    }
                    if (matchesCategory(calleeName, "authentication")) {
                        features.auth_api = true;
                    }
                    if (matchesCategory(calleeName, "filesystem")) {
                        features.file_io = true;
                    }
                    if (matchesCategory(calleeName, "network")) {
                        features.network_io = true;
                    }
                }
            }
        }
    }

    // Check function name itself
    std::string funcName = F.getName().str();
    if (matchesCategory(funcName, "crypto")) {
        features.crypto_api = true;
    }
    if (matchesCategory(funcName, "authentication")) {
        features.auth_api = true;
    }
    if (matchesCategory(funcName, "filesystem")) {
        features.file_io = true;
    }
    if (matchesCategory(funcName, "network")) {
        features.network_io = true;
    }

    // Backward compatibility setting
    features.is_sensitive = features.crypto_api || features.auth_api;
}

} // namespace analysis
