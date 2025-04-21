import os

def parse_env_file(env_path):
    result = []
    with open(env_path, "r") as f:
        for line in f:
            if "=" in line and not line.strip().startswith("#"):
                key, value = line.strip().split("=", 1)
                result.append((key.strip(), value.strip()))
    return result

def generate_header_file(env_vars, output_path):
    with open(output_path, "w") as f:
        f.write("// Auto-generated from .env\n")
        f.write("#pragma once\n\n")
        for key, value in env_vars:
            f.write(f'#define {key} "{value}"\n')

def before_build(source, target, env):
    env_path = ".env"
    header_path = "include/credentials.h"
    if os.path.exists(env_path):
        vars = parse_env_file(env_path)
        generate_header_file(vars, header_path)