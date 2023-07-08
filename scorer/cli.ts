import { Problem, Solution } from "../painter/problem.ts";
import { calcScore } from "./core.ts";

const problemPath = Deno.args[0];
const solutionPath = Deno.args[1];

const problem = JSON.parse(await Deno.readTextFile(problemPath)) as Problem;
const solution = JSON.parse(await Deno.readTextFile(solutionPath)) as Solution;

const score = calcScore(problem, solution);
