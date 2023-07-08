use std::{collections::BTreeMap, path::Path};

use walkdir::WalkDir;

use crate::{is_valid, load_problem, load_solution, score, Problem, Result, Solution};

fn load_all_problems<P: AsRef<Path>>(path: P) -> Result<BTreeMap<String, Problem>> {
    let mut problems = BTreeMap::new();
    for entry in WalkDir::new(path)
        .into_iter()
        .filter_map(|r| r.ok())
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
    Ok(problems)
}

fn load_best_solutions<P: AsRef<Path>>(
    path: P,
    problems: &BTreeMap<String, Problem>,
) -> Result<BTreeMap<String, (f64, Solution)>> {
    let mut best_solutions = BTreeMap::new();
    for entry in WalkDir::new(path)
        .into_iter()
        .filter_map(|r| r.ok())
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
        if score < 0.0 {
            continue;
        }
        let best_score = best_solutions
            .entry(problem_id)
            .or_insert((score, solution.clone()));
        if best_score.0 < score {
            *best_score = (score, solution);
        }
    }
    Ok(best_solutions)
}

pub fn load_best<P: AsRef<Path>>(
    problem_dir: P,
    solution_dir: P,
) -> Result<Vec<(String, Solution, Problem)>> {
    let mut problems = load_all_problems(problem_dir)?;
    let best_solutions = load_best_solutions(solution_dir, &problems)?;

    let mut pairs = vec![];
    for (problem_id, (_, solution)) in best_solutions {
        match problems.remove(&problem_id) {
            Some(problem) => {
                pairs.push((problem_id, solution, problem));
            }
            None => continue,
        }
    }
    Ok(pairs)
}
