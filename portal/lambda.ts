import { Context, APIGatewayProxyResultV2, APIGatewayProxyEventV2 } from 'aws-lambda';
import { GatewayEvent } from "./types";
import { submitSolutionHandler } from "./submit_solution";
import { Pool } from 'pg';

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

export async function handler(
  event: APIGatewayProxyEventV2 & {path: string},
  context: Context,
): Promise<APIGatewayProxyResultV2> {
  console.log(event);
    let result: any;
    if (event.path == '/solutions/submit') {
        result = await submitSolutionHandler(event, context, pg);
    } else {
        const q = await pg.query('SELECT * FROM problems');
        result = {
            size: q.rowCount,
            firstRow: q.rows[0]
        };
    }

  return {
    body: JSON.stringify(result),
    headers: { "content-type": "text/html;charset=utf8" },
    statusCode: 200,
  };
}
