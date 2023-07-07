use rand::{rngs::StdRng, Rng, SeedableRng};

use crate::{problem::Problem, score, solution::Solution, types::P};

const RADIUS: f64 = 10.0;

pub(crate) fn is_valid(problem: &Problem, placements: &[P]) -> bool {
    for p in placements {
        if p.x < problem.stage_bottom_left.0 + RADIUS
            || problem.stage_bottom_left.0 + problem.stage_width - RADIUS < p.x
        {
            return false;
        }
        if p.y < problem.stage_bottom_left.1 + RADIUS
            || problem.stage_bottom_left.1 + problem.stage_height - RADIUS < p.y
        {
            return false;
        }
    }

    let n = placements.len();
    for i in 0..n {
        for j in 0..i {
            if placements[i].d2(&placements[j]) < RADIUS * RADIUS {
                return false;
            }
        }
    }
    true
}

pub(crate) fn solve(problem: &Problem, solution: &Solution) -> Solution {
    let mut placements = solution.placements.clone();
    let mut score = score::score(&problem, &placements);

    let mut rng = StdRng::seed_from_u64(42);
    for _ in 0..200 {
        let i = rng.gen_range(0..placements.len());
        let d = rng.gen_range(-1.0..1.0);
        let to_x = rng.gen_bool(0.5);
        if to_x {
            placements[i].x += d;
        } else {
            placements[i].y += d;
        }

        if !is_valid(problem, &placements) {
            if to_x {
                placements[i].x -= d;
            } else {
                placements[i].y -= d;
            }
            continue;
        }

        let new_score = score::score(&problem, &placements);
        if score > new_score {
            if to_x {
                placements[i].x -= d;
            } else {
                placements[i].y -= d;
            }
            continue;
        }

        eprintln!("{} -> {}", score, new_score);
        score = new_score;
    }
    Solution { placements }
}
