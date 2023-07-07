import { Context } from "aws-lambda";
import { GatewayEvent } from "./types";
import { S3Client, GetObjectCommand, PutObjectCommand } from "@aws-sdk/client-s3";
import { writeFile } from "fs/promises";
import { spawn } from 'child_process';

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
  context: Context
): Promise<any> {
    console.log(event);
    const content = JSON.stringify(event['body-json']);
    const solverName = event.params.querystring['solver'];
    const problemId = event.params.querystring['problem_id'];

    const [problemPath, solutionPath] = await Promise.all([
        downloadProblem(parseInt(problemId)),
        saveSolution(`${solverName}-${problemId}`, content),
    ]);

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

    return {
        "status": "success"
    };
}