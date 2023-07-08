use std::time::Instant;

use rand::{rngs::StdRng, Rng, SeedableRng};

use crate::{is_valid, score, types::P, Problem, Solution};

pub fn simulated_annealing2(problem: &Problem, solution: &Solution) -> (Solution, f64) {
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

        let n = state.placements.len();
        let mut scores = vec![(0, 0.0); n];
        for i in 0..n {
            scores[i] = (i, score::single_score(&problem, &state.placements, i));
        }
        scores.sort_by(|a, b| b.1.partial_cmp(&a.1).unwrap());

        let top = (n / 10).max(10).min(n);

        let mv = if rng.gen_bool(0.2) {
            let i = rng.gen_range(0..top);
            let j = rng.gen_range(0..top);
            let (i, j) = (scores[i].0, scores[j].0);
            Move::Swap { i, j }
        } else {
            let i = rng.gen_range(0..top);
            let i = scores[i].0;
            let d = rng.gen_range(-1.0..1.0);
            let to_x = rng.gen_bool(0.5);
            Move::Shift { i, to_x, d }
        };

        state.move_forward(mv);
        if !state.is_valid(problem) {
            state.move_backward(mv);
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
            state.move_backward(mv);
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
    fn move_forward(&mut self, m: Move) {
        match m {
            Move::Shift { i, to_x, d } => {
                if to_x {
                    self.placements[i].x -= d;
                } else {
                    self.placements[i].y -= d;
                }
            }
            Move::Swap { i, j } => {
                self.placements.swap(i, j);
            }
        }
    }
    fn move_backward(&mut self, m: Move) {
        match m {
            Move::Shift { i, to_x, d } => {
                if to_x {
                    self.placements[i].x += d;
                } else {
                    self.placements[i].y += d;
                }
            }
            Move::Swap { i, j } => {
                self.placements.swap(i, j);
            }
        }
    }

    fn is_valid(&self, problem: &Problem) -> bool {
        is_valid(problem, &self.placements)
    }
}

#[derive(Clone, Copy)]
enum Move {
    Shift { i: usize, to_x: bool, d: f64 },
    Swap { i: usize, j: usize },
}