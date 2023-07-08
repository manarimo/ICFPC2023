use crate::{problem::Problem, types::P};

mod sa;
pub use sa::simulated_annealing;

mod sa2;
pub use sa2::simulated_annealing2;

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
