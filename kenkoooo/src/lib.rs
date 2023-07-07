use std::path::Path;

pub(crate) type Result<T> = std::result::Result<T, Box<dyn std::error::Error>>;
pub(crate) mod problem;
pub(crate) mod score;
pub(crate) mod solution;

pub fn run<P: AsRef<Path>, S: AsRef<Path>>(problem: P, solution: S) -> Result<()> {
    let problem = problem::load_problem(problem)?;
    let solution = solution::load_solution(solution)?;

    let score = score::score(&problem, &solution);
    println!("{}", score);

    Ok(())
}
