import { loadProblem } from "../painter/core.ts";

interface ProblemAttributes {
    id: string;
    roomWidth: number;
    roomHeight: number;
    numMusicians: number;
    numAttendees: number;
    note?: string;
}

function extractProblemId(path: string): string {
    const result = /(\d+)\.json/.exec(path);
    if (result == null || result.length < 2) {
        throw new Error(`Cannot extract problem ID from ${path}`);
    }
    return result[1];
}

const attrs: ProblemAttributes[] = [];
for await (const dirEntry of Deno.readDir('problems')) {
    if (dirEntry.name.endsWith('.json')) {
        const problem = await loadProblem(`problems/${dirEntry.name}`);
        const id = extractProblemId(dirEntry.name);
        attrs.push({
            id: id,
            roomWidth: problem.room_width,
            roomHeight: problem.room_height,
            numMusicians: problem.musicians.length,
            numAttendees: problem.attendees.length,
        });
    }
}

attrs.sort((a, b) => parseInt(a.id) - parseInt(b.id));
Deno.writeTextFileSync("public/problem-index.json", JSON.stringify(attrs));
