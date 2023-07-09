use kenkoooo::{load_best, score, simulated_annealing};
use rayon::prelude::{IntoParallelIterator, ParallelIterator};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();
    let args: Vec<String> = std::env::args().collect();

    let rows = load_best(&args[1], &args[2])?;
    let out = &args[3];
    rows.into_par_iter()
        .for_each(|(problem_id, mut solution, problem)| {
            let mut cur_score = score(&problem, &solution.placements);
            let mut start_temp = std::env::var("START_TEMP")
                .ok()
                .and_then(|s| s.parse::<f64>().ok())
                .unwrap_or(1e6);
            let time_limit = std::env::var("TIME_LIMIT")
                .ok()
                .and_then(|s| s.parse::<f64>().ok())
                .unwrap_or(60.0);
            let mut trial = 0;

            loop {
                let result = simulated_annealing(&problem, &solution, start_temp, time_limit);
                solution = result.0;
                let accepted = result.1;
                let score = score(&problem, &solution.placements);
                if score <= cur_score {
                    trial += 1;
                    if accepted > 0.3 {
                        start_temp /= 2.0;
                    } else {
                        start_temp *= 2.0;
                    }

                    if trial > 4 {
                        break;
                    }
                } else {
                    trial = 0;
                }
                let file = match std::fs::File::create(format!("{out}/{problem_id}.json")) {
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
                let delta = (score - cur_score) / 1e6;
                log::info!("problem={problem_id}:\t{cur_score}\t->\t{score}\tdelta:{delta}*1e6");
                cur_score = score;
            }
        });

    Ok(())
}
