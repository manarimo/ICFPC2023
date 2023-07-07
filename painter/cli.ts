import { processFile } from "./core.ts";
import * as flags from "https://deno.land/std@0.193.0/flags/mod.ts";

if (Deno.args.length < 2) {
    console.error('Usage: deno task painter [problem file] ([solution file]) --outfile=[output file]');
    Deno.exit(1);
}

const args = flags.parse(Deno.args);

const inFiles = args['_'];
const outFile = args['outfile'];

let problemFile: string;
let solutionFile: string | undefined;

if (inFiles.length == 1) {
    problemFile = String(inFiles[0]);
    solutionFile = undefined;
} else if (inFiles.length == 2) {
    problemFile = String(inFiles[0]);
    solutionFile = String(inFiles[1]);
} else {
    console.error('Usage: deno task painter [problem file] ([solution file]) --outfile=[output file]');
    Deno.exit(1);
}

await processFile(problemFile, solutionFile, outFile);