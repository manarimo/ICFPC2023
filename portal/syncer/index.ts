import { Context } from "aws-lambda";
import { S3Util } from "../util/s3";
import { Pool } from "pg";
import { readFile, rm } from "fs/promises";
import { discordSay } from "../util/discord";
import { setTimeout } from "timers/promises";

const dbHost = process.env["POSTGRES_HOST"]!;
const password = process.env["POSTGRES_PASSWORD"]!;
const portalToken = process.env["PORTAL_TOKEN"]!;

const pg = new Pool({
  user: "postgres",
  database: "icfpc2023",
  host: dbHost,
  port: 5432,
  password: password,
});
pg.on("error", (err) => {
  console.error("pg error", err);
});

const s3Util = new S3Util();

async function downloadAndSubmit(problemId: number, solutionS3Path: string): Promise<void> {
    const tmpPath = `/tmp/${problemId}.json`;
    await s3Util.downloadS3Object(solutionS3Path, tmpPath);
    const solution = await readFile(tmpPath);

    for (let i = 0; i < 3; ++i) {
        const response = await fetch('https://api.icfpcontest.com/submission', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Authorization': `Bearer ${portalToken}`
            },
            body: JSON.stringify({
                problem_id: problemId,
                contents: solution.toString()
            })
        });

        const statusClass = Math.floor(response.status / 100);
        if (statusClass != 2) {
            const error = await response.text();
            if (i == 2) {
                throw new Error(`Failed to submit ${problemId}: ${error}`);
            } else {
                console.error(`Failed to submit ${problemId}. Retrying...`);
                await setTimeout(1000);
            }
        } else {
            break;
        }
    }
    console.log(`Submitted ${problemId}`);
    await rm(tmpPath);
}

export async function handler(
  event: any,
  context: Context,
): Promise<void> {
    // Query top solutions
    const solutions = await pg.query(`with g as (
        select *,
                rank() over (partition by problem_id order by score desc, id asc) as r
        from solutions
        )
        select *
        from g
        where r=1;`
    );

    try {
        for (const row of solutions.rows) {
            console.log(`Processing problem ${row['problem_id']} (max = ${row['score']} by ${row['solver_name']})`);
            await downloadAndSubmit(row['problem_id'], row['solution_path']);
        }
    } catch (e) {
        console.error(e);
        await discordSay('提出失敗したじゃも……');
        return;
    }

    await discordSay('一番いいヤツ！提出しといたよ！');
}