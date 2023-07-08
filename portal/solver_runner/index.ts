import { Pool } from "pg";
import { S3Util } from "../util/s3";
import { Context } from "aws-lambda";
import { Spawner } from "../util/spawner";
import { chmod, writeFile } from "fs/promises";

interface SolverRunnerEvent {
    problemId: number;
    solverPath: string;
    solverName: string;
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

async function downloadProblem(problemId: number): Promise<string> {
    const fsPath = `/tmp/problem-${problemId}.json`;
    await s3Util.downloadS3Object(`problems/${problemId}.json`, fsPath);
    return fsPath;
}

export async function handler(
  event: SolverRunnerEvent,
  context: Context,
): Promise<void> {
    // Download solver and problem
    const [solverPath, problemPath] = await Promise.all([
        downloadSolver(event.solverPath),
        downloadProblem(event.problemId)
    ]);

    // Run solver
    const solver = new Spawner(solverPath);
    const solution = await solver.run(problemPath);
    const solutionPath = '/tmp/solution.json';
    await writeFile(solutionPath, solution);

    // Run scorer
    const scorer = new Spawner('/opt/scorer', [problemPath, solutionPath]);
    const score = parseInt(await scorer.run());
    console.log(`Score: ${score}`);

    // Save solution
    const solutionS3Path = `solutions/${event.solverName}/${event.problemId}.json`;
    await s3Util.uploadS3Object(solutionS3Path, solution);

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
