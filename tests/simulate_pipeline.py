#!/usr/bin/env python3
import os
import re

# ---------------------------------------------------------
# Step 1: Simulated C++ Parser & Feature Extractor
# ---------------------------------------------------------

def extract_functions(content):
    # Regex to find function signatures: [type] [name]([args]) {
    # It matches identifiers, references, pointers, templates
    pattern = re.compile(r'(?:[a-zA-Z_][a-zA-Z0-9_<>&:]*\s+)+([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*\{')
    
    functions = {}
    pos = 0
    while True:
        match = pattern.search(content, pos)
        if not match:
            break
        func_name = match.group(1)
        start_idx = match.end() - 1
        
        # Match braces to find the end of the function body
        brace_count = 0
        end_idx = start_idx
        for i in range(start_idx, len(content)):
            if content[i] == '{':
                brace_count += 1
            elif content[i] == '}':
                brace_count -= 1
                if brace_count == 0:
                    end_idx = i + 1
                    break
        
        body = content[start_idx:end_idx]
        functions[func_name] = body
        pos = end_idx
    return functions

def extract_features(file_path):
    with open(file_path, 'r') as f:
        content = f.read()

    # Remove comments to avoid false positive matches
    content_clean = re.sub(r'//.*', '', content)
    content_clean = re.sub(r'/\*.*?\*/', '', content_clean, flags=re.DOTALL)

    functions = extract_functions(content_clean)
    features_list = []

    # First pass: find called functions to build the call graph
    # We want to find user-defined functions called inside other functions
    func_names = set(functions.keys())
    callers = {name: set() for name in func_names}
    callees = {name: set() for name in func_names}

    for func_name, body in functions.items():
        # Find potential function calls: word(
        calls = re.findall(r'\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\(', body)
        for called in calls:
            if called in func_names and called != func_name:
                callees[func_name].add(called)
                callers[called].add(func_name)

    # Second pass: compute structural metrics
    for func_name, body in functions.items():
        if func_name == "main" and len(functions) > 1:
            # We can analyze main or skip/include it
            pass

        # Heuristic for basic blocks count:
        # Base block = 1, each conditional/loop branch splits into blocks
        branches = len(re.findall(r'\b(if|for|while|case|catch)\b', body))
        basic_block_count = 1 + branches * 2

        # Cyclomatic complexity: 1 + number of decision points
        decision_points = len(re.findall(r'\b(if|for|while|case|&&|\|\|)\b', body))
        cyclomatic_complexity = 1 + decision_points

        # CFG density: E / V
        # If V = basic_block_count, E = cyclomatic_complexity + V - 2
        if basic_block_count > 1:
            edge_count = cyclomatic_complexity + basic_block_count - 2
            cfg_density = edge_count / basic_block_count
        else:
            cfg_density = 0.0

        # Instruction diversity: count of distinct token opcodes / operators
        # We can extract distinct operators and keywords in the body
        tokens = re.findall(r'\b(std|cout|vector|int|double|char|bool|string|return|for|while|if|else|static_cast|length|size|push_back)\b|[+\-*/%^&|~=!<>]+', body)
        instruction_diversity = len(set(tokens))
        if instruction_diversity == 0:
            instruction_diversity = 5 # default min

        # Sensitivity Check (matching feature_extraction.cpp logic)
        sensitive_keywords = ["login", "encrypt", "decrypt", "auth", "key", "cipher", "crypto"]
        is_sensitive = any(kw in func_name.lower() for kw in sensitive_keywords)
        
        # Or if it calls a sensitive function
        if not is_sensitive:
            for callee in callees[func_name]:
                if any(kw in callee.lower() for kw in sensitive_keywords):
                    is_sensitive = True
                    break

        features_list.append({
            "function_name": func_name,
            "cyclomatic_complexity": cyclomatic_complexity,
            "basic_block_count": basic_block_count,
            "cfg_density": round(cfg_density, 3),
            "instruction_diversity": instruction_diversity,
            "is_sensitive": is_sensitive,
            "call_graph_fan_in": len(callers[func_name]),
            "call_graph_fan_out": len(callees[func_name])
        })

    return features_list

# ---------------------------------------------------------
# Step 2: Simulated Security & Threat Analyzer (Module 2)
# ---------------------------------------------------------

def analyze_threats(features, profile="balanced"):
    multiplier = 1.0
    if profile == "strict":
        multiplier = 1.3
    elif profile == "lax":
        multiplier = 0.7

    reports = []
    for f in features:
        raw_score = 0.0
        raw_score += f["cyclomatic_complexity"] * 0.02
        raw_score += f["basic_block_count"] * 0.01
        raw_score += f["instruction_diversity"] * 0.005
        raw_score += f["call_graph_fan_out"] * 0.02
        raw_score += f["call_graph_fan_in"] * 0.005

        if f["is_sensitive"]:
            raw_score += 0.40

        risk_score = min(1.0, max(0.0, raw_score * multiplier))

        # Sensitivity category
        name_lower = f["function_name"].lower()
        if f["is_sensitive"]:
            if "login" in name_lower or "auth" in name_lower:
                category = "authentication"
            elif any(x in name_lower for x in ["encrypt", "decrypt", "crypt", "key", "cipher", "crypto"]):
                category = "cryptography"
            else:
                category = "sensitive_generic"
        else:
            category = "generic"

        reports.append({
            "function_name": f["function_name"],
            "risk_score": round(risk_score, 3),
            "sensitivity_category": category
        })
    return reports

# ---------------------------------------------------------
# Step 3: Simulated Protection Planner (Module 3)
# ---------------------------------------------------------

def generate_plan(reports, features):
    plans = []
    HIGH_RISK_THRESHOLD = 0.8

    # Index features by function name
    feat_map = {f["function_name"]: f for f in features}

    for r in reports:
        func_name = r["function_name"]
        risk = r["risk_score"]
        
        if risk >= HIGH_RISK_THRESHOLD:
            selected_passes = ["ControlFlowFlattening", "DecoyIdiomInjection", "SemanticDivergence", "SymbolPoisoning"]
            intensity = 5
            rounds = 3
            target_acc = 0.20
        else:
            selected_passes = ["InstructionSubstitution", "SymbolPoisoning"]
            intensity = 2
            rounds = 1
            target_acc = 0.50

        plans.append({
            "function_name": func_name,
            "selected_passes": selected_passes,
            "intensity_level": intensity,
            "max_transformation_rounds": rounds,
            "target_llm_reconstruction_accuracy": target_acc
        })
    return plans

# ---------------------------------------------------------
# Step 4: Run and Display Output
# ---------------------------------------------------------

def print_table(title, headers, rows):
    print(f"\n=== {title} ===")
    # Calculate column widths
    widths = [len(h) for h in headers]
    for row in rows:
        for idx, val in enumerate(row):
            widths[idx] = max(widths[idx], len(str(val)))

    # Format line
    format_str = " | ".join([f"{{:<{w}}}" for w in widths])
    sep_line = "-+-".join(["-" * w for w in widths])

    print(format_str.format(*headers))
    print(sep_line)
    for row in rows:
        print(format_str.format(*[str(val) for val in row]))

def test_file(file_name, file_path):
    print(f"\n" + "="*80)
    print(f" TESTING FILE: {file_name}")
    print(f" Path: {file_path}")
    print("="*80)

    # 1. Feature Extraction
    features = extract_features(file_path)
    feat_headers = ["Function Name", "Complexity", "BB Count", "CFG Density", "Diversity", "Is Sensitive", "Fan-In", "Fan-Out"]
    feat_rows = [
        [
            f["function_name"],
            f["cyclomatic_complexity"],
            f["basic_block_count"],
            f["cfg_density"],
            f["instruction_diversity"],
            "Yes" if f["is_sensitive"] else "No",
            f["call_graph_fan_in"],
            f["call_graph_fan_out"]
        ]
        for f in features
    ]
    print_table("MODULE 1: Feature Extraction Metrics", feat_headers, feat_rows)

    # 2. Threat Analysis
    threats = analyze_threats(features, profile="balanced")
    threat_headers = ["Function Name", "Risk Score", "Sensitivity Category"]
    threat_rows = [
        [t["function_name"], t["risk_score"], t["sensitivity_category"]]
        for t in threats
    ]
    print_table("MODULE 2: Security & Threat Report", threat_headers, threat_rows)

    # 3. Protection Planning
    plans = generate_plan(threats, features)
    plan_headers = ["Function Name", "Intensity", "Rounds", "Target LLM Acc", "Selected Passes"]
    plan_rows = [
        [
            p["function_name"],
            p["intensity_level"],
            p["max_transformation_rounds"],
            f"{int(p['target_llm_reconstruction_accuracy']*100)}%",
            ", ".join(p["selected_passes"])
        ]
        for p in plans
    ]
    print_table("MODULE 3: Adaptive Protection Plan", plan_headers, plan_rows)

if __name__ == "__main__":
    test_dir = os.path.dirname(os.path.abspath(__file__))
    programs_dir = os.path.join(test_dir, "sample_programs")

    files = {
        "simple.cpp": os.path.join(programs_dir, "simple.cpp"),
        "moderate.cpp": os.path.join(programs_dir, "moderate.cpp"),
        "crypto.cpp": os.path.join(programs_dir, "crypto.cpp")
    }

    for name, path in files.items():
        if os.path.exists(path):
            test_file(name, path)
        else:
            print(f"Error: {path} not found.")
