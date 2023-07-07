use std::path::Path;

use serde::Deserialize;

use crate::Result;

#[derive(Deserialize)]
pub(crate) struct Solution {
    pub(crate) placements: Vec<Place>,
}

#[derive(Deserialize)]
pub(crate) struct Place {
    pub(crate) x: f64,
    pub(crate) y: f64,
}

pub(crate) fn load_solution<P: AsRef<Path>>(path: P) -> Result<Solution> {
    let file = std::fs::File::open(path)?;
    let solution = serde_json::from_reader(file)?;
    Ok(solution)
}
