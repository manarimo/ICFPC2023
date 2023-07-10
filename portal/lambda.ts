import {
  APIGatewayProxyEventV2,
  APIGatewayProxyResultV2,
  Context,
} from "aws-lambda";
import { submitSolutionHandler } from "./submit_solution";
import { Pool } from "pg";
import { handler as buildHandler } from "./builder/build_solver";

const dbHost = process.env["POSTGRES_HOST"]!;
const password = process.env["POSTGRES_PASSWORD"]!;
const discordWebhook = process.env['DISCORD_WEBHOOK']!;

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

export async function handler(
  event: APIGatewayProxyEventV2 & { path: string },
  context: Context,
): Promise<APIGatewayProxyResultV2> {
  console.log(event);
  let result: any;
  if (event.path == "/solutions/submit") {
    result = await submitSolutionHandler(event, context, pg);
  } else if (event.path == "/build") {
    result = await buildHandler(event, context);
  } else if (event.path == "/notify") {
    result = await notificationHandler(event);
  } else {
    result = await apiHandler(event);
  }

  return {
    body: JSON.stringify(result),
    headers: {
      "content-type": "application/json;charset=utf8",
      "Access-Control-Allow-Origin": "*",
    },
    statusCode: 200,
  };
}

async function apiHandler(event: APIGatewayProxyEventV2 & { path: string }) {
  switch (event.path) {
    case "/problems": {
      type ProblemRow = {
        problem_id: number;
        room_width: number;
        room_height: number;
        num_musicians: number;
        num_attendees: number;
      };
      const result = await pg.query<ProblemRow>("SELECT * FROM problems");
      const problems = result.rows.map((
        { problem_id, room_height, room_width, num_attendees, num_musicians },
      ) => ({
        problemId: problem_id,
        roomHeight: room_height,
        roomWidth: room_width,
        numAttendees: num_attendees,
        numMusicians: num_musicians,
      }));
      return {
        problems,
      };
    }
    case "/solutions": {
      type SolutionRow = {
        id: number;
        solver_name: string;
        problem_id: number;
        score: string;
        solution_path: string;
        tag: string;
      };
      const tag = event.queryStringParameters ? event.queryStringParameters['tag'] : undefined;
      let sqlParams: any[] = [];
      let tagFilter = '';
      if (tag) {
        tagFilter = ' where tag=$1 ';
        sqlParams.push(tag);
      }

      const bestIdResult = await pg.query(`with g as (
          select *,
                dense_rank() over (partition by problem_id order by score desc) as r
          from solutions
          ${tagFilter}
          order by problem_id, id
        )
        select min(id) as id, problem_id, r
        from g
        where r<6
        group by problem_id, r
      `, sqlParams);
      const bestIds = bestIdResult.rows.map((r) => r['id']);
      if (bestIds.length == 0) {
        return {
          solutions: []
        };
      }

      const placeholders: string[] = [];
      for (let i = 1; i <= bestIds.length; ++i) {
        placeholders.push('$' + i);
      }
      const whereClause = `where id in (${placeholders.join(',')})`;
      const result = await pg.query<SolutionRow>(`select * from solutions ${whereClause} order by problem_id, score desc`, bestIds);
      const solutions: any[] = [];
      result.rows.forEach(({ id, solver_name, problem_id, score, solution_path, tag }) => {
        const solution = {
          id,
          solverName: solver_name,
          problemId: problem_id,
          score: parseInt(score),
          solutionPath: solution_path,
          tag,
        };
        solutions.push(solution);
      });
      return {
        solutions,
      };
    }
    case "/tags": {
      const result = await pg.query('select distinct(tag) as tag from solutions order by tag');
      return {
        tags: result.rows.map((r) => r['tag']).filter((t) => t != null)
      };
    }
    default: {
      return {
        message: "hello manarimo",
      };
    }
  }
}

async function notificationHandler(event: APIGatewayProxyEventV2 & { path: string }): Promise<void> {
  await fetch(discordWebhook, {
    method: 'POST',
    headers: {
      'Content-Type': "application/json"
    },
    body: event.body
  });
}
