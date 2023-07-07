import {
  APIGatewayProxyEventV2,
  APIGatewayProxyResultV2,
  Context,
} from "https://deno.land/x/lambda@1.32.5/mod.ts";
import { Client } from "https://deno.land/x/postgres@v0.17.0/mod.ts";

const dbHost = Deno.env.get('POSTGRES_HOST')!;
const password = Deno.env.get('POSTGRES_PASSWORD')!;

const pg = new Client({
    user: 'postgres',
    database: 'icfpc2023',
    hostname: dbHost,
    port: 5432,
    password: password
});

export async function handler(
  event: APIGatewayProxyEventV2,
  context: Context,
): Promise<APIGatewayProxyResultV2> {
    const result = await pg.queryObject('SELECT * FROM problems');

    const res = {
        size: result.rowCount,
        firstRow: result.rows[0]
    };

  return {
    body: JSON.stringify(res),
    headers: { "content-type": "text/html;charset=utf8" },
    statusCode: 200,
  };
}
