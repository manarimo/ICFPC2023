import { APIGatewayProxyEventV2, Context } from "aws-lambda";
import { chmod, writeFile } from "fs/promises";
import { Pool as PgPool } from 'pg';
import { S3Util } from "./util/s3";
import { Spawner } from "./util/spawner";
import { paintSolution } from "./paint/paint";

const s3Util = new S3Util();

async function downloadProblem(id: number): Promise<string> {
    const tmppath = `/tmp/problem-tmp-${id}.json`;
    await s3Util.downloadS3Object(`problems/${id}.json`, tmppath);
    return tmppath;
}

async function saveSolution(id: string, content: string): Promise<string> {
    const tmppath = `/tmp/solution-tmp-${id}.json`;
    await writeFile(tmppath, content);
    return tmppath;
}

async function downloadScorer(): Promise<string> {
    const fsPath = `/tmp/scorer`;
    await s3Util.downloadS3Object(`solver/scorer`, fsPath);
    await chmod(fsPath, 0o755);
    return fsPath;
}

export async function submitSolutionHandler(
  event: APIGatewayProxyEventV2,
  context: Context,
  pg: PgPool,
): Promise<any> {
    // Debug
    console.log(event);

    // Validation
    if (event.queryStringParameters == undefined) {
        throw new Error('Query parameters missing');
    }

    // Extract request info
    const content = event.body;
    if (content == undefined) {
        throw new Error('POST body missing');
    }
    const solverName = event.queryStringParameters['solver'];
    if (solverName == undefined) {
        throw new Error('solver param missing');
    }
    const problemId = event.queryStringParameters['problem_id'];
    if (problemId == undefined) {
        throw new Error('problem_id param missing');
    }

    // Save problem and solution on the file system
    const [problemPath, solutionPath] = await Promise.all([
        downloadProblem(parseInt(problemId)),
        saveSolution(`${solverName}-${problemId}`, content),
    ]);

    // Run scorer
    const scorerPath = await downloadScorer();
    const scorer = new Spawner(scorerPath, [problemPath, solutionPath]);
    const score = parseInt(await scorer.run());
    console.log(`score: ${score}`);

    // Generate solution image
    const solutionImagePath = '/tmp/solution.svg';
    await paintSolution(problemPath, solutionPath, solutionImagePath);

    // Upload solution to S3
    const key = `solutions/${solverName}/${problemId}.json`;
    await s3Util.uploadS3Object(key, content);
    console.log(`Stored file as ${key}`);

    // Save solution image
    const solutionImageS3Path = `solutions/${solverName}/images/${problemId}.svg`;
    await s3Util.uploadS3ObjectFromFile(solutionImageS3Path, solutionImagePath, 'image/svg+xml');

    // Update solution database
    const params = [solverName, parseInt(problemId), score, key];
    try {
        console.log(params);
        const response = await pg.query('INSERT INTO solutions (solver_name, problem_id, score, solution_path) VALUES ($1, $2, $3, $4)', params);
        console.log(response);
    } catch (e) {
        console.error(`pg error`, e);
    }

    return {
        "status": "success"
    };
}