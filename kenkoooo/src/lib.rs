use std::path::Path;

pub(crate) type Result<T> = std::result::Result<T, Box<dyn std::error::Error>>;
pub(crate) mod problem;
pub(crate) mod score;
pub(crate) mod solution;
pub(crate) mod solver;
mod types;

pub use problem::{load_problem, Problem};
pub use score::score;
pub use solution::{load_solution, Solution};
pub use solver::{is_valid, simulated_annealing};
pub(crate) use types::P;

pub fn run<P: AsRef<Path>, S: AsRef<Path>>(problem: P, solution: S) -> Result<()> {
    let problem = problem::load_problem(problem)?;
    let mut solution = solution::load_solution(solution)?;

    loop {
        solution = solver::simulated_annealing(&problem, &solution);
        let score = score::score(&problem, &solution.placements);
        let file = std::fs::File::create(format!("solution-{score}.json"))?;
        serde_json::to_writer(file, &solution)?;
        log::info!("score={score}");
    }
}
