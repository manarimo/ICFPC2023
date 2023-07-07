use std::{io::Read, path::Path};

use serde::{Deserialize, Serialize};

use crate::{Result, P};

#[derive(Deserialize, Serialize, Clone)]
pub struct Solution {
    pub placements: Vec<P>,
}

pub fn load_solution<P: AsRef<Path>>(path: P) -> Result<Solution> {
    let mut file = std::fs::File::open(path)?;
    let mut buf = String::new();
    file.read_to_string(&mut buf)?;
    let solution = serde_json::from_str(&buf)?;
    Ok(solution)
}
