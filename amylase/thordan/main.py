import json
import sys
from typing import List, Tuple
import numpy as np
import scipy.optimize


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


def evaluate(problem: Problem, solution: Solution) -> float:
    # todo: implement block
    score = 0
    for musician, placement in zip(problem.musicians, solution.placements):
        for attendee in problem.attendees:
            score += attendee.tastes[musician.instrument] / distance(placement, attendee.position)
    return -score


def solve(problem: Problem) -> Solution:
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
    constraints = []

    def fun_factory(i, j):
        def fun(x):
            return (x[2 * i] - x[2 * j]) ** 2 + (x[2 * i + 1] - x[2 * j + 1]) ** 2 - min_distance ** 2
        return fun
    for j in range(len(problem.musicians)):
        for i in range(j):
            constraints.append({
                "type": "ineq",
                "fun": fun_factory(i, j),
            })

    res = scipy.optimize.minimize(objective, np.zeros((2 * len(problem.musicians), )), bounds=bounds, constraints=constraints, options={"disp": False})
    return arg_to_solution(res.x)


def main():
    problem_dict = json.load(sys.stdin)
    problem = Problem(problem_dict)
    solution = solve(problem)
    solution_dict = solution.to_dict()
    json.dump(solution_dict, sys.stdout)


if __name__ == '__main__':
    main()
