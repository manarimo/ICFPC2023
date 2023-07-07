import { Context, APIGatewayProxyResultV2 } from 'aws-lambda';
import { GatewayEvent } from "./types";
import { submitSolutionHandler } from "./submit_solution";
import { Client } from 'pg';

const dbHost = process.env['POSTGRES_HOST']!;
const password = process.env['POSTGRES_PASSWORD']!;

const pg = new Client({
    user: 'postgres',
    database: 'icfpc2023',
    hostname: dbHost,
    port: 5432,
    password: password
});

export async function handler(
  event: GatewayEvent,
  context: Context,
): Promise<APIGatewayProxyResultV2> {
    let result: any;
    if (event.context["resource-path"] == '/solutions/submit') {
        result = await submitSolutionHandler(event, context);
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
