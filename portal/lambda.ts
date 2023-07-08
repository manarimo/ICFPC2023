import {
  APIGatewayProxyEventV2,
  APIGatewayProxyResultV2,
  Context,
} from "aws-lambda";
import { submitSolutionHandler } from "./submit_solution";
import { Pool } from "pg";

const dbHost = process.env["POSTGRES_HOST"]!;
const password = process.env["POSTGRES_PASSWORD"]!;

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
  } else {
    result = await apiHandler(event);
  }

  return {
    body: JSON.stringify(result),
    headers: { "content-type": "application/json;charset=utf8" },
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
      const result = await pg.query<SolutionRow>("SELECT * FROM solutions");
      const solutions = result.rows.map((
        { id, solver_name, problem_id, score, solution_path },
      ) => ({
        id,
        solverName: solver_name,
        problemId: problem_id,
        score,
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
