#include <iostream>
#include <string>
#include <vector>

// Helper to simulate key derivation/validation
bool validate_key(const std::string& key) {
    if (key.length() < 6) return false;
    unsigned int checksum = 0;
    for (char c : key) {
        checksum += static_cast<unsigned int>(c);
    }
    return (checksum % 7 == 3);
}

// Mimic authentication check (Sensitive name: "auth" and "login")
bool authenticate_user(const std::string& username, const std::string& password) {
    // This is sensitive because it has "auth" in the name and calls validate_key
    std::string mock_secret_key = "SuperSecret123";
    if (username == "admin" && password == "secretpass") {
        return validate_key(mock_secret_key);
    }
    return false;
}

// A complex crypto routine to showcase high risk scoring (Sensitive name: "cipher")
void aes_like_cipher_block(unsigned char* state, const unsigned char* round_keys) {
    for (int round = 0; round < 10; ++round) {
        for (int i = 0; i < 16; ++i) {
            state[i] = (state[i] * 31 + 17) ^ round_keys[round * 16 + i];
            if (state[i] % 2 == 0) {
                state[i] ^= 0x55;
            } else {
                state[i] ^= 0xAA;
            }
            if (state[i] > 100 && state[i] < 200) {
                state[i] = ~state[i];
            }
        }
        unsigned char temp = state[1];
        state[1] = state[5]; state[5] = state[9]; state[9] = state[13]; state[13] = temp;
        for (int col = 0; col < 4; ++col) {
            unsigned char s0 = state[col * 4];
            unsigned char s1 = state[col * 4 + 1];
            unsigned char s2 = state[col * 4 + 2];
            unsigned char s3 = state[col * 4 + 3];
            
            state[col * 4] = (s0 ^ s1) + s2;
            if (s0 > 128) state[col * 4] ^= 0x1B;
            state[col * 4 + 1] = (s1 ^ s2) + s3;
            if (s1 > 128) state[col * 4 + 1] ^= 0x1B;
            state[col * 4 + 2] = (s2 ^ s3) + s0;
            if (s2 > 128) state[col * 4 + 2] ^= 0x1B;
            state[col * 4 + 3] = (s3 ^ s0) + s1;
            if (s3 > 128) state[col * 4 + 3] ^= 0x1B;
        }
    }
}

// Mimic a encryption function (Sensitive name: "encrypt" and "cipher")
std::string encrypt_data(const std::string& plaintext, const std::string& cipher_key) {
    std::string ciphertext = plaintext;
    if (!validate_key(cipher_key)) {
        return "";
    }
    
    // Call the complex cipher block to simulate heavy encryption
    unsigned char mock_state[16] = {0};
    unsigned char mock_round_keys[160] = {0};
    aes_like_cipher_block(mock_state, mock_round_keys);

    for (size_t i = 0; i < plaintext.length(); ++i) {
        ciphertext[i] = plaintext[i] ^ cipher_key[i % cipher_key.length()];
    }
    return ciphertext;
}

// Mimic a decryption function (Sensitive name: "decrypt" and "cipher")
std::string decrypt_data(const std::string& ciphertext, const std::string& cipher_key) {
    return encrypt_data(ciphertext, cipher_key); // XOR is symmetric
}

int main() {
    std::string username = "admin";
    std::string password = "secretpass";
    
    if (authenticate_user(username, password)) {
        std::cout << "User authenticated successfully!" << std::endl;
        
        std::string secret_message = "This is a highly sensitive message!";
        std::string key = "MySecretKey_3";
        
        std::string encrypted = encrypt_data(secret_message, key);
        std::cout << "Encrypted: " << encrypted << std::endl;
        
        std::string decrypted = decrypt_data(encrypted, key);
        std::cout << "Decrypted: " << decrypted << std::endl;
    } else {
        std::cout << "Authentication failed." << std::endl;
    }
    
    return 0;
}
