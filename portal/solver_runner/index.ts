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
}

const dbHost = process.env['POSTGRES_HOST']!;
const password = process.env['POSTGRES_PASSWORD']!;

const pg = new Pool({
    user: 'postgres',
    database: 'icfpc2023',
    host: dbHost,
    port: 5432,
    password: password
});
pg.on('error', (err) => {
  console.error('pg error', err);
});

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
    let seedToUse = seed;
    if (seed == '__best__') {
        const bestSeed = await determineBestSeed(problemId);
        if (bestSeed == null) {
            return null;
        }
        seedToUse = bestSeed;
    }
    console.log(`Using ${seedToUse} as seed solution`);
    await s3Util.downloadS3Object(`solutions/${seedToUse}/${problemId}.json`, fsPath);
    return fsPath;
}

async function determineBestSeed(problemId: number): Promise<string | null> {
    const response = await pg.query('SELECT solver_name FROM solutions WHERE problem_id = $1 ORDER BY score DESC LIMIT 1', [problemId]);
    if (response.rowCount == 0) {
        return null;
    }
    return response.rows[0]['solver_name'];
}

export async function handler(
  event: SolverRunnerEvent,
  context: Context,
): Promise<void> {
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

    // Save score info
    const params = [event.solverName, event.problemId, score, solutionS3Path];
    console.log(params);
    const response = await pg.query(
        'INSERT INTO solutions (solver_name, problem_id, score, solution_path)'
        + ' VALUES ($1, $2, $3, $4)'
        + ' ON CONFLICT ON CONSTRAINT solutions_solver_name_problem_id_key'
        + ' DO UPDATE SET score=$3, solution_path=$4 ', params);
    console.log(response);
}
