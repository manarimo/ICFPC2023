use std::ops::Sub;

use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Copy, Deserialize, Serialize)]
pub(crate) struct P {
    pub(crate) x: f64,
    pub(crate) y: f64,
}

impl P {
    pub(crate) fn arg(&self) -> f64 {
        self.y.atan2(self.x)
    }
    pub(crate) fn abs(&self) -> f64 {
        (self.x * self.x + self.y * self.y).sqrt()
    }
    pub(crate) fn d2(&self, rhs: &Self) -> f64 {
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
