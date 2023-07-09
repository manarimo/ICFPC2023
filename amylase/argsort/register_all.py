import subprocess
import time


solution_name = f"synced_best_{int(time.time())}"
print(f"solution name = {solution_name}")
for i in range(1, 91):
    print(f"problem {i}")
    subprocess.run(f"curl -X POST 'https://vwbqm1f1u5.execute-api.ap-northeast-1.amazonaws.com/prod/solutions/submit?solver={solution_name}&problem_id={i}' -d  @'/home/amylase/repo/ICFPC2023/solutions/synced-bests/{i}.json'", shell=True)
