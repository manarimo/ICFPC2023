use std::{io::Read, path::Path};

use serde::Deserialize;

use crate::Result;

#[derive(Deserialize, Clone)]
pub struct Problem {
    // pub(crate) room_width: f64,
    // pub(crate) room_height: f64,
    pub stage_width: f64,
    pub stage_height: f64,
    pub stage_bottom_left: (f64, f64),
    pub musicians: Vec<usize>,
    pub attendees: Vec<Attendee>,
    pub pillars: Vec<Pillar>,
    pub playing_together: Option<bool>,
}

#[derive(Deserialize, Clone)]
pub struct Attendee {
    pub x: f64,
    pub y: f64,
    pub tastes: Vec<f64>,
}

#[derive(Deserialize, Clone)]
pub struct Pillar {}

pub fn load_problem<P: AsRef<Path>>(path: P) -> Result<Problem> {
    let mut file = std::fs::File::open(path)?;
    let mut buf = String::new();
    file.read_to_string(&mut buf)?;
    let problem: Problem = serde_json::from_str(&buf)?;
    Ok(problem)
}
