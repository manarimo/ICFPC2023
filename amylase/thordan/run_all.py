from pathlib import Path
import subprocess


for problem_id in range(1, 46):
    print(problem_id)
    try:
        subprocess.call(f"venv/bin/python3 main.py < ../../problems/{problem_id}.json > ../../solutions/thordan/{problem_id}.json", shell=True, timeout=10)
        print("solved.")
    except subprocess.TimeoutExpired as e:
        print("time out")
        subprocess.call(f"rm ../../solutions/thordan/{problem_id}.json", shell=True)
