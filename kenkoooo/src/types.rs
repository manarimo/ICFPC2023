use std::ops::Sub;

use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Copy, Deserialize, Serialize)]
pub struct P {
    pub x: f64,
    pub y: f64,
}

impl P {
    pub fn arg(&self) -> f64 {
        self.y.atan2(self.x)
    }
    pub fn abs(&self) -> f64 {
        (self.x * self.x + self.y * self.y).sqrt()
    }
    pub fn d2(&self, rhs: &Self) -> f64 {
        (self.x - rhs.x) * (self.x - rhs.x) + (self.y - rhs.y) * (self.y - rhs.y)
    }
}

impl Sub for P {
    type Output = P;

    fn sub(self, rhs: Self) -> Self::Output {
        P {
            x: self.x - rhs.x,
            y: self.y - rhs.y,
        }
    }
}
