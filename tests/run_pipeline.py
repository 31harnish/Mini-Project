#!/usr/bin/env python3
"""run_pipeline.py

A lightweight replacement for the previous simulated pipeline.
It accepts a C/C++ source file, compiles it to LLVM IR using clang++,
extracts basic function information from the generated .ll file, and
produces a simple table of dummy metrics and selected obfuscation passes.

Usage:
    python run_pipeline.py path/to/source.cpp

The script expects `clang++` to be available in the PATH.
"""

import argparse
import pathlib
import re
import subprocess
import sys
from typing import List, Dict

def compile_to_ir(source_path: pathlib.Path) -> pathlib.Path:
    """Compile the given C/C++ source file to LLVM IR (.ll).

    The IR file is written next to the source file with the same stem and
    a .ll extension. clang++ is invoked with -S -emit-llvm -O2.
    """
    if not source_path.is_file():
        sys.stderr.write(f"Error: source file {source_path} does not exist\n")
        sys.exit(1)
    ir_path = source_path.with_suffix('.ll')
    cmd = ["clang++", "-S", "-emit-llvm", "-O2", str(source_path), "-o", str(ir_path)]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        sys.stderr.write("Compilation failed:\n")
        sys.stderr.write(result.stderr)
        sys.exit(result.returncode)
    return ir_path

def main():
    parser = argparse.ArgumentParser(description="Run the obfuscation pipeline on a C/C++ source file or an existing LLVM IR (.ll) file.")
    parser.add_argument("input_path", type=pathlib.Path, help="Path to the C/C++ source file or LLVM IR (.ll) file")
    args = parser.parse_args()

    if args.input_path.suffix == ".ll":
        ir_path = args.input_path
    else:
        ir_path = compile_to_ir(args.input_path)
    
    # Extra step for IR generation with the existing framework
    framework_bin = pathlib.Path(__file__).parent.parent / "build" / "obfuscation_framework"
    if framework_bin.exists():
        print(f"\nExtra Step: Running actual obfuscation framework on {ir_path}...")
        result = subprocess.run([str(framework_bin), str(ir_path)], capture_output=True, text=True)
        if result.returncode == 0:
            print("Obfuscation complete. Framework output:")
            print(result.stdout)
        else:
            print("Obfuscation framework failed:")
            print(result.stderr)
    else:
        print(f"\nWarning: Actual framework binary not found at {framework_bin}")

if __name__ == "__main__":
    main()
