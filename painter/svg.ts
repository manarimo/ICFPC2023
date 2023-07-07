export interface Circle {
    x: number;
    y: number;
    radius: number;
    fill?: string;
    stroke?: string;
}

export interface Rectangle {
    x: number;
    y: number;
    width: number;
    height: number;
    fill?: string;
    stroke?: string;
}

export class Svg {
    private _circles: Circle[] = [];
    private _rects: Rectangle[] = [];

    constructor(readonly width: number, readonly height: number) {
    }

    circle(c: Circle) {
        this._circles.push(c);
    }

    rect(r: Rectangle) {
        this._rects.push(r);
    }

    get circles(): Circle[] {
        return this._circles;
    }

    get rects(): Rectangle[] {
        return this._rects;
    }
}