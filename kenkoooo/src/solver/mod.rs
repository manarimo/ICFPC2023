use std::time::Instant;

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

pub(crate) fn simulated_annealing(problem: &Problem, solution: &Solution) -> Solution {
    let placements = solution.placements.clone();
    let score = score::score(&problem, &placements);

    const TIME_LIMIT: f64 = 60.0;
    const START_TEMP: f64 = 1e4;
    const END_TEMP: f64 = 1.0;

    let mut rng = StdRng::seed_from_u64(42);
    let mut state = State { score, placements };
    let mut best = state.clone();
    let start_time = Instant::now();
    let mut prev = Instant::now();
    let mut total_trial = 0.0;
    let mut accepted = 0.0;
    while start_time.elapsed().as_secs_f64() < TIME_LIMIT {
        if prev.elapsed().as_secs() > 3 {
            prev = Instant::now();
            log::info!(
                "best_score={} accepted={}",
                best.score,
                accepted / total_trial
            );
        }

        let i = rng.gen_range(0..state.placements.len());
        let d = rng.gen_range(-1.0..1.0);
        let to_x = rng.gen_bool(0.5);
        state.move_forward(i, to_x, d);
        if !state.is_valid(problem) {
            state.move_back(i, to_x, d);
            continue;
        }

        let new_score = score::score(&problem, &state.placements);
        let temp =
            START_TEMP + (END_TEMP - START_TEMP) * start_time.elapsed().as_secs_f64() / TIME_LIMIT;
        let prob = ((new_score - state.score) / temp).exp();

        total_trial += 1.0;
        if prob > rng.gen_range(0.0..1.0) {
            accepted += 1.0;
            state.score = new_score;
            if new_score > best.score {
                best = state.clone();
            }
        } else {
            state.move_back(i, to_x, d);
        }
    }
    Solution {
        placements: best.placements,
    }
}

#[derive(Clone)]
struct State {
    score: f64,
    placements: Vec<P>,
}

impl State {
    fn move_forward(&mut self, i: usize, to_x: bool, d: f64) {
        if to_x {
            self.placements[i].x -= d;
        } else {
            self.placements[i].y -= d;
        }
    }
    fn move_back(&mut self, i: usize, to_x: bool, d: f64) {
        if to_x {
            self.placements[i].x += d;
        } else {
            self.placements[i].y += d;
        }
    }

    fn is_valid(&self, problem: &Problem) -> bool {
        is_valid(problem, &self.placements)
    }
}