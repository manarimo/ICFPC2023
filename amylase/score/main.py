import sys
import subprocess
from pathlib import Path


script_dir = Path(__file__).parent
repositry_root = script_dir.parent.parent
binary_path = script_dir / "judge.exe"


def ensure_judge_binary():
    library_path = repositry_root / "library"
    judge_source_path = script_dir / "main.cpp"
    if not binary_path.exists():
        subprocess.run(["c++", "-std=c++20", "-I" + str(library_path), "-O2", str(judge_source_path), "-o", str(binary_path)])


def main():
    if len(sys.argv) < 3:
        print(f"usage: {sys.argv[0]} problem solution")
        return

    ensure_judge_binary()
    subprocess.run([str(binary_path), sys.argv[1], sys.argv[2]])


if __name__ == '__main__':
    main()