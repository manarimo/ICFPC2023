import { Pool } from "pg";
import { S3Util } from "../util/s3";
import { Context } from "aws-lambda";
import { Spawner } from "../util/spawner";
import { chmod, writeFile } from "fs/promises";
import { paintSolution } from "../paint/paint";
import { SpawnOptionsWithoutStdio } from "child_process";

export interface SolverRunnerEvent {
    problemId: number;
    solverPath: string;
    solverName: string;
    seed?: string;
    env?: Record<string, string>;
    tag?: string;
}

export interface SolverRunnerResponse {
    problemId: number;
    solverName: string;
    score: number;
    tag?: string;
}

const s3Util = new S3Util();

async function downloadSolver(path: string): Promise<string> {
    const fsPath = `/tmp/solver`;
    await s3Util.downloadS3Object(path, fsPath);
    await chmod(fsPath, 0o755);
    return fsPath;
}

async function downloadScorer(): Promise<string> {
    const fsPath = `/tmp/scorer`;
    await s3Util.downloadS3Object(`solver/scorer`, fsPath);
    await chmod(fsPath, 0o755);
    return fsPath;
}

async function downloadProblem(problemId: number): Promise<string> {
    const fsPath = `/tmp/problem-${problemId}.json`;
    await s3Util.downloadS3Object(`problems/${problemId}.json`, fsPath);
    return fsPath;
}

async function downloadSeed(problemId: number, seed?: string): Promise<string | null> {
    if (seed == undefined) {
        return null;
    }

    const fsPath = `/tmp/seed-${problemId}.json`;
    await s3Util.downloadS3Object(`solutions/${seed}/${problemId}.json`, fsPath);
    return fsPath;
}

export async function handler(
  event: SolverRunnerEvent,
  context: Context,
): Promise<SolverRunnerResponse> {
    // Download solver, scorer and problem
    const [solverPath, scorerPath, problemPath, seedPath] = await Promise.all([
        downloadSolver(event.solverPath),
        downloadScorer(),
        downloadProblem(event.problemId),
        downloadSeed(event.problemId, event.seed),
    ]);

    // Run solver
    let solverArgs: string[] = [];
    if (seedPath != null) {
        solverArgs.push(seedPath);
    }
    let options: SpawnOptionsWithoutStdio = {};
    if (event.env) {
        options.env = {
            ...process.env,
            ...event.env
        };
    }
    const solver = new Spawner(solverPath, solverArgs, options);
    const solution = await solver.run(problemPath);
    const solutionPath = '/tmp/solution.json';
    await writeFile(solutionPath, solution);

    // Run scorer
    const scorer = new Spawner(scorerPath, [problemPath, solutionPath]);
    const score = parseInt(await scorer.run());
    console.log(`Score: ${score}`);

    // Generate solution image
    const solutionImagePath = '/tmp/solution.svg';
    await paintSolution(problemPath, solutionPath, solutionImagePath);

    // Save solution
    const solutionS3Path = `solutions/${event.solverName}/${event.problemId}.json`;
    await s3Util.uploadS3Object(solutionS3Path, solution);

    // Save solution image
    const solutionImageS3Path = `solutions/${event.solverName}/images/${event.problemId}.svg`;
    await s3Util.uploadS3ObjectFromFile(solutionImageS3Path, solutionImagePath, 'image/svg+xml');

    return {
        problemId: event.problemId,
        solverName: event.solverName,
        score: score,
        tag: event.tag,
    };
}
