import { processFile } from "./painter/core.ts";

for (let i = 1; i <= 45; ++i) {
    await processFile(`problems/${i}.json`, `problems/images/${i}.svg`);
}