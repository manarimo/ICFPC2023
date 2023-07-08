import { S3Util } from "../util/s3";
import { APIGatewayProxyEventV2, APIGatewayProxyResultV2, Context } from "aws-lambda";
import { Spawner } from "../util/spawner";
import { execSync } from "child_process";
import { rm, stat } from "fs/promises";
import { statSync } from "fs";

interface BuildSolverPayload {
    path: string;
    solverName: string;
}

const s3Util = new S3Util();

export async function handler(
  event: APIGatewayProxyEventV2,
  context: Context,
): Promise<APIGatewayProxyResultV2> {
    try {
        if (event.body == undefined) {
            throw new Error('Missing body');
        }
        const payload = JSON.parse(event.body);

        // Clone Git repository
        console.log('Cloning repo');
        const prefix = '/tmp/ICFPC2023';

        if (statSync(prefix, { throwIfNoEntry: false }) != undefined) {
            await rm(prefix, { recursive: true });
        }
        process.env.GIT_SSH_COMMAND = 'ssh -i /opt/icfpc2023-deploy-key -o StrictHostKeyChecking=no ';
        execSync(`git clone --depth=1 git@github.com:manarimo/ICFPC2023.git ${prefix}`);
        console.log('Done.');

        // Run build.sh in the specified directory
        console.log('Building binary');
        const build = new Spawner(
            './build.sh',
             [],
             {cwd: `${prefix}/${payload.path}`}
        );
        const buildOutput = await build.run();
        console.log(buildOutput);

        // Copy the output artifact to S3
        await s3Util.uploadS3ObjectFromFile(`solver/${payload.solverName}`, `${prefix}/${payload.path}/a.out`);
    } catch(e) {
        if (e instanceof Error) {
            return {
                statusCode: 500,
                body: e.message
            };
        } else {
            throw e;
        }
    }
    return {
        statusCode: 200,
        body: 'success'
    };
}
