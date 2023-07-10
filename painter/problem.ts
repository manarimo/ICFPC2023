export interface Attendee {
    x: number;
    y: number;
    tastes: number[];
}

export interface Pillar {
    center: [number, number];
    radius: number;
}

export interface Problem {
    room_width: number;
    room_height: number;
    stage_width: number;
    stage_height: number;
    stage_bottom_left: [number, number];
    musicians: number[];
    attendees: Attendee[];
    pillars: Pillar[];
}

export interface Point {
    x: number;
    y: number;
}

export interface Solution {
    placements: Point[];
    volumes?: number;
}