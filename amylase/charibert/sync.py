from pathlib import Path
import subprocess
import shutil


script_dir = Path(__file__).parent.resolve()
repositry_root = script_dir.parent.parent


def score(problem_file: Path, solution_file: Path) -> int:
    result = subprocess.run([
        "python3", 
        str(repositry_root / "amylase" / "score" / "main.py"), 
        str(problem_file), 
        str(solution_file),
    ], capture_output=True)
    return int(result.stdout)


def main():
    best_dir = repositry_root / "solutions" / "synced-bests"
    delta = 0
    synced = 0
    for sync_solution_file in best_dir.iterdir():
        if not sync_solution_file.name.endswith(".json"):
            continue
        problem_id = sync_solution_file.name.replace(".json", "")
        problem_file = repositry_root / "problems" / f"{problem_id}.json"

        my_solution_file = repositry_root / "solutions" / "charibert" / f"{problem_id}.json"
        sync_score = score(problem_file, sync_solution_file)
        my_score = score(problem_file, my_solution_file)
        if sync_score > my_score:
            print(f"syncing {problem_id}: {my_score} -> {sync_score}")
            shutil.copy(sync_solution_file, my_solution_file)
            synced += 1
        else:
            print(f"not syncing {problem_id}: {my_score} > {sync_score} (+{my_score - sync_score})")
            delta += my_score - sync_score
    print(f"the next submission will improve the global score by {delta}")
    print(f"synced {synced} files")



if __name__ == '__main__':
    main()
