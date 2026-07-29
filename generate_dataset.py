import random
import string

def random_string(length=16):
    chars = string.ascii_letters + string.digits + "!@#$%^&*"
    return '"' + ''.join(random.choices(chars, k=length)) + '"'

def generate_benign(idx):
    lines = []
    lines.append(f"int benign_func_{idx}(int a, int b) {{")
    lines.append("    int result = a + b;")
    
    # Randomly add a small loop
    if random.random() > 0.5:
        lines.append("    for(int i=0; i<a; ++i) {")
        lines.append("        result += i;")
        lines.append("    }")
        
    lines.append('    const char* label = "simple debug info";')
    lines.append("    return result;")
    lines.append("}")
    return "\n".join(lines)

def generate_sensitive(idx):
    lines = []
    lines.append(f"void crypto_malware_{idx}(const char* input) {{")
    
    # Insert high-entropy string keys
    if random.random() > 0.1:
        lines.append(f"    const char* secret_key = {random_string(16)};")
    
    # Randomly nested loops (depth 1 to 3)
    depth = random.randint(1, 3)
    for i in range(depth):
        lines.append("    " * (i+1) + f"for(int j{i}=0; j{i}<5; ++j{i}) {{")
    
    # Inject Crypto/Network APIs
    lines.append("    " * (depth+1) + "int dummy_var = 0;")
    if random.random() > 0.2:
        lines.append("    " * (depth+1) + "extern void encrypt_data(const char*); encrypt_data(input);")
    if random.random() > 0.2:
        lines.append("    " * (depth+1) + "extern void validate_key(const char*); validate_key(input);")
        
    for i in range(depth-1, -1, -1):
        lines.append("    " * (i+1) + "}")
        
    lines.append("}")
    return "\n".join(lines)

with open("synthetic_dataset.cpp", "w") as f:
    for i in range(500):
        f.write(generate_benign(i) + "\n\n")
    for i in range(500):
        f.write(generate_sensitive(i) + "\n\n")

print("Generated synthetic_dataset.cpp with 1000 functions!")
