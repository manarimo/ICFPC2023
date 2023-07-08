import { Attendee, Point, Problem, Solution } from "../painter/problem.ts";

const BLOCK_DISTANCE = 5.0;

export function calcScore(problem: Problem, solution: Solution) {
  let totalScore = 0;
  for (let i = 0; i < problem.musicians.length; i++) {
    totalScore += singleScore(problem, solution.placements, i);
  }
  return totalScore;
}

function singleScore(
  problem: Problem,
  placements: Point[],
  musicianId: number,
) {
  let score = 0;
  const attendees = unblockedAttendees(problem, placements, musicianId);
  for (const attendee of attendees) {
    const musician = placements[musicianId];
    const taste = attendee.tastes[problem.musicians[musicianId]];
    score += Math.ceil(1000000 * taste / dist2(attendee, musician));
  }
  return score;
}

function unblockedAttendees(
  problem: Problem,
  placements: Point[],
  musicianId: number,
) {
  const unblockedAttendees = [] as Attendee[];
  const center = placements[musicianId];

  type Event = { kind: "Start" } | { kind: "End" } | {
    kind: "Attendee";
    attendee: Attendee;
  };

  const events = [] as { argument: number; info: number }[];
  const eventInfos = [] as Event[];
  for (const attendee of problem.attendees) {
    const pos = attendee;
    const argument = arg(sub(pos, center));
    events.push({ argument, info: eventInfos.length });
    eventInfos.push({ kind: "Attendee", attendee });
  }

  let overlappingSpans = 0;
  for (let i = 0; i < placements.length; i++) {
    if (musicianId === i) continue;

    const location = placements[i];
    const v = sub(location, center);
    const distance = abs(v);
    const offset = Math.asin(BLOCK_DISTANCE / distance);

    const argument = arg(v);
    const start = normalizeAngle(argument - offset);
    const end = normalizeAngle(argument + offset);
    if (start > end) overlappingSpans += 1;

    events.push({ argument: start, info: eventInfos.length });
    eventInfos.push({ kind: "Start" });
    events.push({ argument: end, info: eventInfos.length });
    eventInfos.push({ kind: "End" });
  }

  events.sort((a, b) => a.argument - b.argument);

  for (const event of events) {
    const info = eventInfos[event.info];
    switch (info.kind) {
      case "Start":
        overlappingSpans += 1;
        break;
      case "End":
        overlappingSpans -= 1;
        break;
      case "Attendee":
        if (overlappingSpans === 0) {
          unblockedAttendees.push(info.attendee);
        }
        break;
    }
  }
  return unblockedAttendees;
}

function sub(a: Point, b: Point) {
  return {
    x: a.x - b.x,
    y: a.y - b.y,
  };
}

function arg(p: Point) {
  return Math.atan2(p.y, p.x);
}

function abs(p: Point) {
  return Math.sqrt(p.x * p.x + p.y * p.y);
}

function normalizeAngle(a: number) {
  while (a < -Math.PI) a += 2 * Math.PI;
  while (a > Math.PI) a -= 2 * Math.PI;
  return a;
}

function dist2(p: Point, q: Point) {
  return (p.x - q.x) * (p.x - q.x) + (p.y - q.y) * (p.y - q.y);
}
