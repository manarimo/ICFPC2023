import { Context } from "aws-lambda";
import { GatewayEvent } from "./types";
import { S3Client, PutObjectCommand } from "@aws-sdk/client-s3";

export async function submitSolutionHandler(
  event: GatewayEvent,
  context: Context
): Promise<any> {
    console.log(event);
    const content = event['body-json'];
    const solverName = event.params.querystring['solver'];
    const problemId = event.params.querystring['problem_id'];

    const key = `solutions/${solverName}/${problemId}.json`;
    const s3 = new S3Client({});
    const command = new PutObjectCommand({
        Bucket: "icfpc2023-manarimo-3mrzsd",
        Key: key,
        Body: JSON.stringify(content),
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