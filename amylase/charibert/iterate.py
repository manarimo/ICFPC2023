import subprocess
import random
import multiprocessing
import sys
from pathlib import Path


script_dir = Path(__file__).parent.resolve()
repositry_root = script_dir.parent.parent
binary_path = script_dir / "iterate.exe"


target_type = "lightning"


def update(id: int):
    key = random.randint(1, 10000000)
    try:
        subprocess.run(f"{str(binary_path)} ../../solutions/charibert/{id}.json < ../../problems/{id}.json > {id}_{key}.json && mv {id}_{key}.json ../../solutions/charibert/{id}.json", shell=True)
    except Exception as _e:
        subprocess.run(f"rm {id}_{key}.json", shell=True)


def ensure_binary():
    library_path = repositry_root / "library"
    judge_source_path = script_dir / ("main.cpp" if target_type == "lightning" else "block_pillar.cpp")
    subprocess.run(["c++", "-std=c++17", "-I" + str(library_path), "-O2", str(judge_source_path), "-o", str(binary_path)])


def id_filter(problem_id: int) -> bool:
    if target_type == "lightning":
        return 1 <= problem_id <= 55
    else:
        return 56 <= problem_id <= 90


def main():
    global target_type
    target_dir = Path(sys.argv[1])
    if len(sys.argv) >= 3:
        target_type = sys.argv[2].strip()

    ensure_binary()

    problem_ids = []
    for file in target_dir.iterdir():
        if not file.name.endswith(".json"):
            continue
        problem_ids.append(int(file.name.replace(".json", "")))
    problem_ids = [i for i in problem_ids if id_filter(i)]

    while True:
        weights = {}
        for id in problem_ids:
            result = subprocess.run([
                "python3", 
                str(repositry_root / "amylase" / "score" / "main.py"), 
                str(repositry_root / "problems" / f"{id}.json"), 
                str(target_dir / f"{id}.json"),
                # "--skip-validate"
            ], capture_output=True)
            score = int(result.stdout)
            weight = max(score, 0)
            if weight > 0:
                weights[id] = weight
        
        concurrency = min(multiprocessing.cpu_count() - 1, len(weights))
        target_ids = []
        for _ in range(concurrency):
            to_choice = list(weights.keys())
            target = random.choices(to_choice, weights=[weights[i] for i in to_choice])[0]
            target_ids.append(target)
            del weights[target]

        print(target_ids)
        with multiprocessing.Pool() as pool:
            pool.map(update, target_ids)


if __name__ == '__main__':
    main()
