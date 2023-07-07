use std::collections::BTreeMap;

use kenkoooo::{is_valid, load_problem, load_solution, score, simulated_annealing};
use rayon::prelude::{IntoParallelIterator, ParallelIterator};
use walkdir::WalkDir;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();
    let args: Vec<String> = std::env::args().collect();

    let mut problems = BTreeMap::new();
    for entry in WalkDir::new(&args[1])
        .into_iter()
        .filter_map(Result::ok)
        .filter(|e| e.file_type().is_file())
    {
        let path = entry.path();
        if path.extension().and_then(|s| s.to_str()) != Some("json") {
            continue;
        }
        log::info!("loading {:?}", path);
        let problem_id = match path.file_stem().and_then(|s| s.to_str()) {
            Some(stem) => stem.to_string(),
            None => continue,
        };

        let problem = load_problem(path)?;
        problems.insert(problem_id, problem);
    }
    log::info!("loaded {} problems", problems.len());

    let mut best_solutions = BTreeMap::new();
    for entry in WalkDir::new(&args[2])
        .into_iter()
        .filter_map(Result::ok)
        .filter(|e| e.file_type().is_file())
    {
        let path = entry.path();
        if path.extension().and_then(|s| s.to_str()) != Some("json") {
            continue;
        }
        log::info!("loading {:?}", path);
        let problem_id = match path.file_stem().and_then(|s| s.to_str()) {
            Some(stem) => stem.to_string(),
            None => continue,
        };
        let problem = match problems.get(&problem_id) {
            Some(problem) => problem,
            None => continue,
        };

        let solution = match load_solution(path) {
            Ok(solution) => solution,
            Err(e) => {
                log::error!("failed to load {:?}: {:?}", path, e);
                continue;
            }
        };
        if !is_valid(problem, &solution.placements) {
            continue;
        }

        let score = score(problem, &solution.placements);
        let best_score = best_solutions
            .entry(problem_id)
            .or_insert((score, solution.clone()));
        if best_score.0 < score {
            *best_score = (score, solution);
        }
    }

    let mut rows = vec![];
    for (problem_id, (_, solution)) in best_solutions {
        match problems.remove(&problem_id) {
            Some(problem) => {
                rows.push((problem_id, solution, problem));
            }
            None => continue,
        }
    }

    rows.into_par_iter()
        .for_each(|(problem_id, mut solution, problem)| {
            let mut cur_score = score(&problem, &solution.placements);
            loop {
                solution = simulated_annealing(&problem, &solution);
                let score = score(&problem, &solution.placements);
                if score <= cur_score {
                    break;
                }
                let file = match std::fs::File::create(format!(
                    "../solutions/kenkoooo-sa-improve/{problem_id}.json"
                )) {
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
                log::info!("problem={problem_id}:\t{cur_score}\t->\t{score}");
                cur_score = score;
            }
        });

    Ok(())
}
