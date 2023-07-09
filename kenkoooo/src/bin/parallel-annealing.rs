use kenkoooo::{load_problem, load_solution, score, simulated_annealing};
use rayon::prelude::{IntoParallelIterator, ParallelIterator};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();
    let args: Vec<String> = std::env::args().collect();

    let size = args[1].parse::<usize>()?;
    let problem = load_problem(&args[2])?;
    let mut solution = load_solution(&args[3])?;

    let mut cur_score = score(&problem, &solution.placements);
    loop {
        let states = (0..size).map(|_| solution.clone()).collect::<Vec<_>>();
        let (new_solution, score) = states
            .into_par_iter()
            .map(|solution| {
                let (solution, accepted) = simulated_annealing(&problem, &solution);
                let score = score(&problem, &solution.placements);
                let delta = (score - cur_score) / 1e6;
                log::info!("{cur_score}\t->\t{score}\tdelta:{delta}*1e6\taccepted:{accepted}");
                (solution, score)
            })
            .max_by_key(|(_, score)| *score as i64)
            .unwrap();

        solution = new_solution;
        if score <= cur_score {
            break;
        }
        let file = match std::fs::File::create(&args[4]) {
            Ok(file) => file,
            Err(e) => {
                log::error!("failed to open solution: {:?}", e);
                break;
            }
        };
        if let Err(e) = serde_json::to_writer(file, &solution) {
            log::error!("failed to write solution: {:?}", e);
            break;
        }
        cur_score = score;
    }

    Ok(())
}
