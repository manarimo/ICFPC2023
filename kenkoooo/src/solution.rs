use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::{Result, P};

#[derive(Deserialize, Serialize)]
pub(crate) struct Solution {
    pub(crate) placements: Vec<P>,
}

pub(crate) fn load_solution<P: AsRef<Path>>(path: P) -> Result<Solution> {
    let file = std::fs::File::open(path)?;
    let solution = serde_json::from_reader(file)?;
    Ok(solution)
}
