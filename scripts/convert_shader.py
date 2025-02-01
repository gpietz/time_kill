import os
import subprocess
import sys

def compile_glsl_to_spirv(directory):
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(('.frag', '.vert')):
                input_path = os.path.join(root, file)
                output_path = input_path + ".spv"

                print(f"Compiling {input_path} -> {output_path}")

                try:
                    subprocess.run(["glslangvalidator", "-V", input_path, "-o", output_path], check=True)
                    print(f"Successfully compiled: {output_path}")
                except subprocess.CalledProcessError as e:
                    print(f"Failed to compile {input_path}: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <directory>")
        sys.exit(1)

    directory = sys.argv[1]
    if os.path.isdir(directory):
        compile_glsl_to_spirv(directory)
    else:
        print("Invalid directory path.")
