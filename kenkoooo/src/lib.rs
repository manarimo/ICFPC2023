pub(crate) type Result<T> = std::result::Result<T, Box<dyn std::error::Error>>;
mod io;
pub(crate) mod problem;
pub(crate) mod score;
pub(crate) mod solution;
pub(crate) mod solver;
mod types;

pub use io::load_best;
pub use problem::{load_problem, Problem};
pub use score::{score, unblocked_pairs};
pub use solution::{load_solution, Solution};
pub use solver::{is_valid, simulated_annealing};
pub use types::P;
