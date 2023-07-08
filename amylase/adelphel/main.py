import json
import subprocess
from typing import List


def get_removal_attendees(problem_id: int) -> List[int]:
    result = subprocess.run(["./a.out", f"../../problems/{problem_id}.json"], capture_output=True, text=True)
    attendee_ids = []
    for line in result.stdout.splitlines():
        attendee_ids.append(int(line))
    return attendee_ids


def main():
    for problem_id in range(56, 91):
        attendee_ids = set(get_removal_attendees(problem_id))
        with open(f"../../problems/{problem_id}.json") as f:
            problem = json.load(f)
        with open(f"../../problems/{problem_id}.json.orig", "w") as f:
            json.dump(problem, f)
        new_attendees = []
        for i, attendee in enumerate(problem["attendees"]):
            if i not in attendee_ids:
                new_attendees.append(attendee)
        problem["attendees"] = new_attendees
        with open(f"../../problems/{problem_id}.json", "w") as f:
            json.dump(problem, f)



if __name__ == "__main__":
    main()