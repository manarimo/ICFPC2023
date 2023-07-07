use std::path::Path;

use serde::Deserialize;

use crate::Result;

#[derive(Deserialize)]
pub(crate) struct Problem {
    // pub(crate) room_width: f64,
    // pub(crate) room_height: f64,
    pub(crate) stage_width: f64,
    pub(crate) stage_height: f64,
    pub(crate) stage_bottom_left: (f64, f64),
    pub(crate) musicians: Vec<usize>,
    pub(crate) attendees: Vec<Attendee>,
}

#[derive(Deserialize)]
pub(crate) struct Attendee {
    pub(crate) x: f64,
    pub(crate) y: f64,
    pub(crate) tastes: Vec<f64>,
}

pub(crate) fn load_problem<P: AsRef<Path>>(path: P) -> Result<Problem> {
    let file = std::fs::File::open(path)?;
    let problem: Problem = serde_json::from_reader(file)?;
    Ok(problem)
}
