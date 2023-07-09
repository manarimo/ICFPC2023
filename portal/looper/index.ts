import { Context } from "aws-lambda";
import { SolverRunnerEvent, SolverRunnerResponse } from "../solver_runner";
import { Pool } from "pg";
import { discordSay } from "../util/discord";

interface LooperEvent {
    label: string;
    count: number;
    lightningSolver?: string;
    pillarSolver?: string;
    limit?: number;
    env?: Record<string, string>;
    exclude?: number[];
    response?: (SolverRunnerResponse | SolverErrorResponse)[];
}

interface SolverErrorResponse {
    Status: "FAILED";
}

type LooperResponseToMap = {
    items: SolverRunnerEvent[];
} & LooperEvent;

interface LooperResponseToEnd {
    end: true
}

type LooperResponse = LooperResponseToMap | LooperResponseToEnd;

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

async function determineBestSeeds(): Promise<Record<number, string>> {
    const response = await pg.query(`
        with g as (
            select *,
            rank() over (partition by problem_id order by score desc, id asc) as r
            from solutions
        )
        select *
        from g
        where r=1; 
    `);

    const result: Record<number, string> = {};
    for (const row of response.rows) {
        result[row['problem_id']] = row['solver_name'];
    }
    return result;
}

async function saveResponse(responses: (SolverRunnerResponse | SolverErrorResponse)[]): Promise<void> {
    const placeholders: string[] = [];
    const values: any[] = [];
    const successfulResponses = responses.filter((r) => r != null && !('Status' in r)) as SolverRunnerResponse[];
    successfulResponses.forEach((r, i) => {
        placeholders.push(`($${i*4 + 1}, $${i*4 + 2}, $${i*4 + 3}, $${i*4 + 4})`);
        values.push(r.solverName);
        values.push(r.problemId);
        values.push(r.score);
        values.push(`solutions/${r.solverName}/${r.problemId}.json`);
    })

    const res = await pg.query(
        'INSERT INTO solutions (solver_name, problem_id, score, solution_path)'
        + ' VALUES '
        + placeholders.join(',')
        + ' ON CONFLICT ON CONSTRAINT solutions_solver_name_problem_id_key'
        + ' DO UPDATE SET score=excluded.score, solution_path=excluded.solution_path ', values);
    console.log(res);
}

export async function handler(
  event: LooperEvent,
  context: Context,
): Promise<LooperResponse> {
    if (event.response) {
        await saveResponse(event.response);
    }

    if (event.limit && event.count > event.limit) {
        await discordSay(`${event.label}が終わったんじゃも！`);
        return {
            end: true
        };
    }

    // Make Kolog work
    await discordSay(`${event.label}のループ${event.count}回目をじっこうします！`);

    const bestSeeds = await determineBestSeeds();

    const items: SolverRunnerEvent[] = [];
    for (let i = 1; i <= 90; ++i) {
        if (event.exclude) {
            if (event.exclude.indexOf(i) != -1) {
                continue;
            }
        }
        let solverPath: string | undefined = undefined;
        if (i <= 55) {
            if (event.lightningSolver) {
                solverPath = `solver/${event.lightningSolver}`;
            }
        } else {
            if (event.pillarSolver) {
                solverPath = `solver/${event.pillarSolver}`;
            }
        }

        if (solverPath) {
            const item: SolverRunnerEvent = {
                problemId: i,
                solverPath: solverPath,
                solverName: `${event.label}-${event.count}`,
                env: event.env
            };
            if (bestSeeds[i]) {
                item.seed = bestSeeds[i];
            }
            items.push(item);
        }
    }

    return {
        ...event,
        count: event.count + 1,
        items
    };
}