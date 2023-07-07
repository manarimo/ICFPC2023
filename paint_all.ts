import { processFile } from "./painter/core.ts";

for (let i = 1; i <= 55; ++i) {
    await processFile(`problems/${i}.json`, undefined, `public/problem_images/${i}.svg`);
}