import { serializeSvg } from "./codec.ts";
import { Problem } from "./problem.ts";
import { Svg } from "./svg.ts";

async function load(inFile: string): Promise<Problem> {
    const json = await Deno.readTextFile(inFile);
    return JSON.parse(json) as Problem;
}

export async function processFile(inFile: string, outFile: string): Promise<void> {
    console.log(`Loading ${inFile}...`);
    const problem = await load(inFile);

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
        y: problem.stage_bottom_left[1],
        width: problem.stage_width,
        height: problem.stage_height,
    });

    // Draw attendees
    for (const attendee of problem.attendees) {
        svg.circle({
            x: attendee.x,
            y: attendee.y,
            radius: 5,
            fill: 'green'
        });
    }

    // Write out SVG file
    console.log(`Writing out to ${outFile}...`);
    const fh = await Deno.open(outFile, { create: true, truncate: true, write: true });
    await serializeSvg(svg, fh.writable);
    fh.close();

    console.log(`Done!`);
}