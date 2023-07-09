use std::collections::BTreeSet;

use kenkoooo::{
    load_problem, load_solution, score, simulated_annealing, unblocked_pairs, Problem, Solution, P,
};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();
    let args: Vec<String> = std::env::args().collect();

    let original_problem = load_problem(&args[1])?;
    let mut solution = load_solution(&args[2])?;

    let pruned_problem = simplify_problem(&original_problem, &solution);
    let mut cur_score = score(&original_problem, &solution.placements);
    loop {
        let start_temp = std::env::var("START_TEMP")
            .ok()
            .and_then(|s| s.parse::<f64>().ok())
            .unwrap_or(1e4);
        let time_limit = std::env::var("TIME_LIMIT")
            .ok()
            .and_then(|s| s.parse::<f64>().ok())
            .unwrap_or(60.0);
        let result = simulated_annealing(&pruned_problem, &solution, start_temp, time_limit);
        solution = result.0;
        let score = score(&original_problem, &solution.placements);
        let delta = (score - cur_score) / 1e6;
        log::info!(
            "{cur_score}\t->\t{score}\tdelta:{delta}*1e6\taccepted:{}",
            result.1
        );
        if score <= cur_score {
            break;
        }
        let file = match std::fs::File::create(&args[3]) {
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

fn simplify_problem(problem: &Problem, solution: &Solution) -> Problem {
    let unblocked_pairs = unblocked_pairs(&problem, &solution.placements);
    let n = problem.attendees.len();
    let mut scores = vec![0.0; n];
    for (i_attendee, i_musician) in unblocked_pairs {
        let attendee = &problem.attendees[i_attendee];
        let musician = &solution.placements[i_musician];
        let instrument = problem.musicians[i_musician];
        let a_pos = P {
            x: attendee.x,
            y: attendee.y,
        };
        let m_pos = P {
            x: musician.x,
            y: musician.y,
        };

        let taste = attendee.tastes[instrument];
        scores[i_attendee] += (1000000.0 * taste / a_pos.d2(&m_pos)).ceil();
    }

    let mut scores = scores.into_iter().enumerate().collect::<Vec<_>>();
    scores.sort_by(|a, b| b.1.partial_cmp(&a.1).unwrap());

    let top_score = scores[0].1;
    let extracted = scores
        .into_iter()
        .filter(|&(_, score)| score > top_score / 1000.)
        .map(|(id, _)| id)
        .collect::<BTreeSet<_>>();

    let mut new_problem = problem.clone();
    new_problem.attendees = new_problem
        .attendees
        .into_iter()
        .enumerate()
        .filter(|(i, _)| extracted.contains(i))
        .map(|(_, a)| a)
        .collect::<Vec<_>>();
    new_problem
}
