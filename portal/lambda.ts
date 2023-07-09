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
        score: number;
        solution_path: string;
      };
      const query=`with g as (
        select *,
               rank() over (partition by problem_id order by score desc, id asc) as r
        from solutions
      )
      select *
      from g
      where r=1;`;
      const result = await pg.query<SolutionRow>(query);
      const solutions = result.rows.map((
        { id, solver_name, problem_id, score, solution_path },
      ) => ({
        id,
        solverName: solver_name,
        problemId: problem_id,
        score: Number(score),
        solutionPath: solution_path,
      }));
      return {
        solutions,
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
