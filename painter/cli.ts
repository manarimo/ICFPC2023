import { processFile } from "./core.ts";

if (Deno.args.length < 2) {
    console.error('Usage: deno task painter [input file] [output file]');
    Deno.exit(1);
}

const inFile = Deno.args[0];
const outFile = Deno.args[1];

await processFile(inFile, outFile);