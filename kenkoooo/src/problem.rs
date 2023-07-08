use std::{io::Read, path::Path};

use serde::Deserialize;

use crate::Result;

#[derive(Deserialize)]
pub struct Problem {
    // pub(crate) room_width: f64,
    // pub(crate) room_height: f64,
    pub(crate) stage_width: f64,
    pub(crate) stage_height: f64,
    pub(crate) stage_bottom_left: (f64, f64),
    pub(crate) musicians: Vec<usize>,
    pub(crate) attendees: Vec<Attendee>,
    pub pillars: Vec<Pillar>,
    pub playing_together: Option<bool>,
}

#[derive(Deserialize)]
pub struct Attendee {
    pub(crate) x: f64,
    pub(crate) y: f64,
    pub(crate) tastes: Vec<f64>,
}

#[derive(Deserialize)]
pub struct Pillar {}

pub fn load_problem<P: AsRef<Path>>(path: P) -> Result<Problem> {
    let mut file = std::fs::File::open(path)?;
    let mut buf = String::new();
    file.read_to_string(&mut buf)?;
    let problem: Problem = serde_json::from_str(&buf)?;
    Ok(problem)
}
