import { serializeSvg } from "./codec.ts";
import { Problem, Solution } from "./problem.ts";
import { Svg } from "./svg.ts";

export async function loadProblem(inFile: string): Promise<Problem> {
    const json = await Deno.readTextFile(inFile);
    return JSON.parse(json) as Problem;
}

async function loadSolution(inFile: string): Promise<Solution> {
    const json = await Deno.readTextFile(inFile);
    return JSON.parse(json) as Solution;
}

export async function processFile(problemFile: string, solutionFile: string | undefined, outFile: string): Promise<void> {
    console.log(`Loading ${problemFile}...`);
    const problem = await loadProblem(problemFile);

    let solution = undefined;
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
        let idx = 0;
        for (const p of solution.placements) {
            const volume = solution.volumes ? solution.volumes[idx] : 1;
            let color: string;
            if (volume > 7) {
                color = 'blue';
            } else if (volume > 4) {
                color = 'cyan';
            } else {
                color = 'lightblue';
            }
            svg.circle({
                x: p.x,
                y: problem.room_height - p.y,
                radius: 5,
                fill: color
            });
            ++idx;
        }
    }

    // Write out SVG file
    console.log(`Writing out to ${outFile}...`);
    const fh = await Deno.open(outFile, { create: true, truncate: true, write: true });
    await serializeSvg(svg, fh.writable);
    fh.close();

    console.log(`Done!`);
}
