import json
import sys
from typing import List, Optional, Tuple
import numpy as np
import scipy.optimize
import itertools


Point = Tuple[float, float]


def point_dict(point: Point):
    return {"x": point[0], "y": point[1]}


class Rectangle:
    def __init__(self, left_bottom: Point, right_top: Point):
        self.left_bottom = left_bottom
        self.right_top = right_top


class Musician:
    def __init__(self, instrument: int) -> None:
        self.instrument = instrument


class Attendee:
    def __init__(self, attendee_dict) -> None:
        self.position = (attendee_dict["x"], attendee_dict["y"])
        self.tastes = attendee_dict["tastes"]


class Problem:
    def __init__(self, problem_dict):
        self.room = Rectangle((0, 0), (problem_dict["room_width"], problem_dict["room_height"]))
        
        stage_left_bottom: List[float] = problem_dict["stage_bottom_left"]
        stage_left, stage_bottom = stage_left_bottom
        stage_top = stage_bottom + problem_dict["stage_height"]
        stage_right = stage_left + problem_dict["stage_width"]
        self.stage = Rectangle((stage_left, stage_bottom), (stage_right, stage_top))

        self.musicians = [Musician(musician) for musician in problem_dict["musicians"]]
        self.attendees = [Attendee(attendee) for attendee in problem_dict["attendees"]]


class Solution:
    def __init__(self, placements: List[Point]) -> None:
        self.placements = placements

    def to_dict(self):
        return {
            "placements": [point_dict(point) for point in self.placements]
        }


def square_distance(p: Point, q: Point) -> float:
    return ((p[0] - q[0]) ** 2 + (p[1] - q[1]) ** 2)


def distance(p: Point, q: Point) -> float:
    return square_distance(p, q) ** 0.5


def score(problem: Problem, solution: Solution) -> float:
    # todo: implement block
    score = 0
    for musician, placement in zip(problem.musicians, solution.placements):
        for attendee in problem.attendees:
            score += attendee.tastes[musician.instrument] / square_distance(placement, attendee.position)
    return score


def evaluate(problem: Problem, solution: Solution) -> float:
    penalty = -score(problem, solution)

    # penalty
    penalty_threshold = 15
    penalty_weight = 0.4
    for placement1, placement2 in itertools.combinations(solution.placements, r=2):
        penalty += penalty_weight * max(penalty_threshold - distance(placement1, placement2), 0)
    return penalty


def solve(problem: Problem, solution: Optional[Solution] = None) -> Solution:
    def arg_to_solution(x: np.ndarray) -> Solution:
        placements = []
        for i in range(len(problem.musicians)):
            placements.append((x[2 * i], x[2 * i + 1]))
        return Solution(placements=placements)

    def objective(x: np.ndarray):
        solution = arg_to_solution(x)
        return evaluate(problem, solution)

    min_distance = 10
    bounds = []
    for _ in problem.musicians:
        bounds.append((problem.stage.left_bottom[0] + min_distance, problem.stage.right_top[0] - min_distance))
        bounds.append((problem.stage.left_bottom[1] + min_distance, problem.stage.right_top[1] - min_distance))

    if solution is not None:
        vec = []
        for placement in solution.placements:
            vec += list(placement)
        initial = np.array(vec)
    else:
        initial = np.zeros((2 * len(problem.musicians), ))
    res = scipy.optimize.minimize(objective, initial, bounds=bounds, options={"disp": True})
    print(f"score = {score(problem, arg_to_solution(res.x))}, penalty = {evaluate(problem, arg_to_solution(res.x))}", file=sys.stderr)
    for m1, m2 in itertools.combinations(range(len(problem.musicians)), r=2):
        x = res.x
        dsq = (x[2 * m1] - x[2 * m2]) ** 2 + (x[2 * m1 + 1] - x[2 * m2 + 1]) ** 2 
        if dsq < min_distance ** 2:
            print(f"constraint violation: {m1} {m2}, d = {dsq ** 0.5}", file=sys.stderr)

    return arg_to_solution(res.x)


def main():
    problem_dict = json.load(sys.stdin)
    problem = Problem(problem_dict)
    if len(sys.argv) >= 2:
        with open(sys.argv[1]) as f:
            intermediate_solution_dict = json.load(f)
            placements = [(placement["x"], placement["y"]) for placement in intermediate_solution_dict["placements"]]
            intermediate_solution = Solution(placements=placements)
    else:
        intermediate_solution = None
    solution = solve(problem, intermediate_solution)
    solution_dict = solution.to_dict()
    json.dump(solution_dict, sys.stdout)


if __name__ == '__main__':
    main()
