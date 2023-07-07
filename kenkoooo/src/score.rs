use std::ops::Sub;

use crate::{problem::Problem, solution::Solution};

const BLOCK_DISTANCE: f64 = 5.0;

pub(crate) fn score(problem: &Problem, solution: &Solution) -> f64 {
    let mut total_score = 0.0;
    for i_musician in 0..solution.placements.len() {
        total_score += single_score(problem, solution, i_musician);
    }
    total_score
}

pub(crate) fn single_score(problem: &Problem, solution: &Solution, i_musician: usize) -> f64 {
    let mut score = 0.0;
    let attendees = unblocked_attendees(problem, solution, i_musician);
    for i_attendee in attendees {
        let attendee = P {
            x: problem.attendees[i_attendee].x,
            y: problem.attendees[i_attendee].y,
        };
        let musician = P {
            x: solution.placements[i_musician].x,
            y: solution.placements[i_musician].y,
        };

        let taste = problem.attendees[i_attendee].tastes[problem.musicians[i_musician]];
        score += (1000000.0 * taste / attendee.d2(&musician)).ceil();
    }

    score
}
fn unblocked_attendees(problem: &Problem, solution: &Solution, i_musician: usize) -> Vec<usize> {
    let mut unblocked = vec![];
    let center = P {
        x: solution.placements[i_musician].x,
        y: solution.placements[i_musician].y,
    };

    #[derive(Debug, Clone, Copy)]
    enum Event {
        Start,
        Attendee(usize),
        End,
    }

    let mut event_info = vec![];
    let mut events = vec![];
    for (i_attendee, attendee) in problem.attendees.iter().enumerate() {
        let pos = P {
            x: attendee.x,
            y: attendee.y,
        };
        let arg = (pos - center).arg();
        events.push((arg, event_info.len()));
        event_info.push(Event::Attendee(i_attendee));
    }

    let mut overlapping_spans = 0;
    for (j_musician, musician) in solution.placements.iter().enumerate() {
        if i_musician == j_musician {
            continue;
        }

        let location = P {
            x: musician.x,
            y: musician.y,
        };
        let v = location - center;
        let distance = v.abs();
        let offset = (BLOCK_DISTANCE / distance).asin();

        let arg = v.arg();
        let start = normalize_angle(arg - offset);
        let end = normalize_angle(arg + offset);
        if start > end {
            overlapping_spans += 1;
        }
        events.push((start, event_info.len()));
        event_info.push(Event::Start);
        events.push((end, event_info.len()));
        event_info.push(Event::End);
    }

    events.sort_unstable_by(|a, b| {
        if a.0 == b.0 {
            a.1.cmp(&b.1)
        } else {
            a.0.partial_cmp(&b.0).unwrap()
        }
    });
    for event in events {
        let event_info = event_info[event.1];
        match event_info {
            Event::Start => {
                overlapping_spans += 1;
            }
            Event::End => {
                overlapping_spans -= 1;
            }
            Event::Attendee(i_attendee) => {
                if overlapping_spans == 0 {
                    unblocked.push(i_attendee);
                }
            }
        }
    }
    unblocked
}

fn normalize_angle(mut angle: f64) -> f64 {
    while angle < -std::f64::consts::PI {
        angle += 2.0 * std::f64::consts::PI;
    }
    while angle > std::f64::consts::PI {
        angle -= 2.0 * std::f64::consts::PI;
    }
    angle
}

#[derive(Debug, Clone, Copy)]
pub(crate) struct P {
    pub(crate) x: f64,
    pub(crate) y: f64,
}

impl P {
    fn arg(&self) -> f64 {
        self.y.atan2(self.x)
    }
    fn abs(&self) -> f64 {
        (self.x * self.x + self.y * self.y).sqrt()
    }
    fn d2(&self, rhs: &Self) -> f64 {
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
