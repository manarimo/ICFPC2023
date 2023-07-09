use crate::{problem::Problem, P};

const BLOCK_DISTANCE: f64 = 5.0;

pub fn score(problem: &Problem, placements: &[P]) -> f64 {
    let mut total_score = 0.0;
    let unblocked_pairs = unblocked_pairs(problem, placements);
    for (i_attendee, i_musician) in unblocked_pairs {
        let attendee = &problem.attendees[i_attendee];
        let musician = &placements[i_musician];
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
        total_score += (1000000.0 * taste / a_pos.d2(&m_pos)).ceil();
    }
    total_score
}

pub fn unblocked_pairs<'a>(problem: &'a Problem, placements: &'a [P]) -> Vec<(usize, usize)> {
    let mut unblocked_pairs = vec![];
    for (i_musician, musician) in placements.iter().enumerate() {
        let center = P {
            x: musician.x,
            y: musician.y,
        };

        #[derive(Clone, Copy)]
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
        for (j_musician, musician) in placements.iter().enumerate() {
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
                        unblocked_pairs.push((i_attendee, i_musician));
                    }
                }
            }
        }
    }
    unblocked_pairs
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
