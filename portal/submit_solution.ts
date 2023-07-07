import { Context } from "aws-lambda";
import { GatewayEvent } from "./types";
import { S3Client, GetObjectCommand, PutObjectCommand } from "@aws-sdk/client-s3";
import { writeFile } from "fs/promises";
import { spawn } from 'child_process';
import { Pool as PgPool } from 'pg';

const s3 = new S3Client({});

async function downloadProblem(id: number): Promise<string> {
    const command = new GetObjectCommand({
        Bucket: "icfpc2023-manarimo-3mrzsd",
        Key: `problems/${id}.json`
    });
    const result = await s3.send(command);
    const tmppath = `/tmp/problem-tmp-${id}.json`;
    const content = await result.Body?.transformToString();
    if (content == undefined) {
        throw new Error(`Cannot read problem file ${id}`);
    }
    await writeFile(tmppath, content);

    return tmppath;
}

async function saveSolution(id: string, content: string): Promise<string> {
    const tmppath = `/tmp/solution-tmp-${id}.json`;
    await writeFile(tmppath, content);
    return tmppath;
}

export async function submitSolutionHandler(
  event: GatewayEvent,
  context: Context,
  pg: PgPool,
): Promise<any> {
    // Debug
    console.log(event);

    // Extract request info
    const content = JSON.stringify(event['body-json']);
    const solverName = event.params.querystring['solver'];
    const problemId = event.params.querystring['problem_id'];

    // Save problem and solution on the file system
    const [problemPath, solutionPath] = await Promise.all([
        downloadProblem(parseInt(problemId)),
        saveSolution(`${solverName}-${problemId}`, content),
    ]);

    // Run scorer
    // /opt/scorer is provided by a Lambda layer.
    // It's a x86_64 binary built from amylase/scorer/score.cpp
    const scorer = spawn('/opt/scorer', [problemPath, solutionPath]);
    const score = await new Promise((resolve, reject) => {
        scorer.stdout.on('data', (data) => {
            console.log(`Scorer printed ${data}`);
            resolve(parseInt(data));
        });
        scorer.stderr.on('data', (data) => {
            console.error(data);
        });
    });

    console.log(`score: ${score}`);

    // Upload solution to S3
    const key = `solutions/${solverName}/${problemId}.json`;
    const command = new PutObjectCommand({
        Bucket: "icfpc2023-manarimo-3mrzsd",
        Key: key,
        Body: content,
    })
    try {
        const result = await s3.send(command);
        console.log(result);
        console.log(`Stored file as ${key}`);
    } catch (e) {
        console.error(e);
    }

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