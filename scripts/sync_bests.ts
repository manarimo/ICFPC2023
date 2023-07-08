const API = "https://api.icfpcontest.com";
const LIMIT = 200;

if (Deno.args.length < 1) {
  console.error("Usage: deno task sync-bests [dir to save solutions]");
  Deno.exit(1);
}
const solutionsDir = Deno.args[0];
const apiKey = Deno.env.get("ICFPC_TOKEN");
if (!apiKey) {
  throw new Error("ICFPC_TOKEN is not set");
}

const bestScores = new Map<number, { score: number; id: string }>();

let offset = 0;
while (true) {
  type ResponseBody = {
    Success: {
      _id: string;
      problem_id: number;
      submitted_at: string;
      score: "Processing" | {
        "Failure": string | undefined;
        "Success": number | undefined;
      };
    }[];
    Failure: string | undefined;
  };

  const url = `${API}/submissions?offset=${offset}&limit=${LIMIT}`;
  console.log(`Fetching ${url} ...`);
  const response = await fetch(url, {
    headers: {
      "Authorization": `Bearer ${apiKey}`,
    },
  });
  const body: ResponseBody = await response.json();
  if (body.Failure) {
    throw new Error(body.Failure);
  }

  if (body.Success.length === 0) break;

  for (const submission of body.Success) {
    if (
      submission.score === "Processing" || submission.score.Failure ||
      !submission.score.Success
    ) {
      continue;
    }
    const score = submission.score.Success;
    const current = bestScores.get(submission.problem_id);
    if (!current || score > current.score) {
      bestScores.set(
        submission.problem_id,
        {
          id: submission._id,
          score,
        },
      );
    }
  }

  offset += LIMIT;
}

for (const [problemId, { id }] of bestScores.entries()) {
  type ResponseBody = {
    Success: {
      contents: string;
    };
    Failure: string | undefined;
  };

  const url = `${API}/submission?submission_id=${id}`;
  console.log(`Fetching ${url} ...`);
  const response = await fetch(url, {
    headers: {
      "Authorization": `Bearer ${apiKey}`,
    },
  });

  const body: ResponseBody = await response.json();
  if (body.Failure) {
    console.error(`Failed to fetch ${url}: ${body.Failure}`);
    continue;
  }

  const contents = body.Success.contents;
  Deno.writeTextFileSync(`${solutionsDir}/${problemId}.json`, contents);
}
