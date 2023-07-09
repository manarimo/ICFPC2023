import { Problem, Solution } from "../types";
import { readFile, writeFile } from 'fs/promises';
import { Svg } from "./svg";
import { createWriteStream } from "fs";
import { serializeSvg } from "./codec";

export async function loadProblem(inFile: string): Promise<Problem> {
    const buffer = await readFile(inFile);
    return JSON.parse(buffer.toString()) as Problem;
}

async function loadSolution(inFile: string): Promise<Solution> {
    const buffer = await readFile(inFile);
    return JSON.parse(buffer.toString()) as Solution;
}

export async function paintSolution(problemFile: string, solutionFile: string | undefined, outFile: string): Promise<void> {
    console.log(`Loading ${problemFile}...`);
    const problem = await loadProblem(problemFile);

    let solution: Solution | undefined = undefined;
    if (solutionFile) {
        console.log(`Loading ${solutionFile}...`);
        solution = await loadSolution(solutionFile);
    }

    console.log(`Converting into SVG...`);
    const svg = new Svg(problem.room_width, problem.room_height);

    // Draw room
    svg.rect({
        x: 0,
        y: 0,
        width: problem.room_width,
        height: problem.room_height,
        fill: 'lightyellow'
    });

    // Draw stage boundary
    svg.rect({
        x: problem.stage_bottom_left[0],
        y: problem.room_height - problem.stage_bottom_left[1] - problem.stage_height,
        width: problem.stage_width,
        height: problem.stage_height,
        stroke: 'black',
        fill: 'white',
    });

    // Draw attendees
    for (const attendee of problem.attendees) {
        svg.circle({
            x: attendee.x,
            y: problem.room_height - attendee.y,
            radius: 5,
            fill: 'green'
        });
    }

    // Draw pillars
    for (const pillar of problem.pillars) {
        svg.circle({
            x: pillar.center[0],
            y: problem.room_height - pillar.center[1],
            radius: pillar.radius,
            fill: 'red'
        });
    }

    if (solution) {
        // Draw solutions
        for (const p of solution.placements) {
            svg.circle({
                x: p.x,
                y: problem.room_height - p.y,
                radius: 5,
                fill: 'blue'
            });
        }
    }

    // Write out SVG file
    console.log(`Writing out to ${outFile}...`);
    const fh = await createWriteStream(outFile);
    await serializeSvg(svg, fh);
    fh.close();
}
