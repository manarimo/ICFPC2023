use std::time::Instant;

use rand::{rngs::StdRng, Rng, SeedableRng};

use crate::{problem::Problem, score, solution::Solution, types::P};

const RADIUS: f64 = 10.0;

pub fn is_valid(problem: &Problem, placements: &[P]) -> bool {
    if problem.musicians.len() != placements.len() {
        return false;
    }
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

pub fn simulated_annealing(problem: &Problem, solution: &Solution) -> (Solution, f64) {
    let placements = solution.placements.clone();
    let score = score::score(&problem, &placements);

    let start_temp = std::env::var("START_TEMP")
        .ok()
        .and_then(|s| s.parse::<f64>().ok())
        .unwrap_or(1e4);
    let time_limit = std::env::var("TIME_LIMIT")
        .ok()
        .and_then(|s| s.parse::<f64>().ok())
        .unwrap_or(60.0);
    log::info!("start_temp={} time_limit={}", start_temp, time_limit);
    const END_TEMP: f64 = 1.0;

    let mut rng = StdRng::seed_from_u64(42);
    let mut state = State { score, placements };
    let mut best = state.clone();
    let start_time = Instant::now();
    let mut prev = Instant::now();
    let mut total_trial = 0.0;
    let mut accepted = 0.0;
    while start_time.elapsed().as_secs_f64() < time_limit {
        if prev.elapsed().as_secs() > 3 {
            prev = Instant::now();
            log::debug!(
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
            start_temp + (END_TEMP - start_temp) * start_time.elapsed().as_secs_f64() / time_limit;
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
    (
        Solution {
            placements: best.placements,
        },
        accepted / total_trial,
    )
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
