use kenkoooo::{load_problem, load_solution, score};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();
    let args: Vec<String> = std::env::args().collect();

    let problem = load_problem(&args[1])?;
    let solution = load_solution(&args[2])?;
    let score = score(&problem, &solution.placements);
    println!("{}", score);
    Ok(())
}
