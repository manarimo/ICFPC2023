function ensureDirectory(path: string): void {
  let stat: Deno.FileInfo;
  try {
    stat = Deno.statSync(path);
  } catch (e) {
    if (e instanceof Deno.errors.NotFound) {
      Deno.mkdir(path);
      return;
    } else {
      throw e;
    }
  }

  if (stat.isDirectory) {
    return;
  }

  throw new Error(`${path} is expected to be a directory`);
}

function extractProblemId(path: string): string {
  const result = /(\d+)\.json/.exec(path);
  if (result == null || result.length < 2) {
    throw new Error(`Cannot extract problem ID from ${path}`);
  }
  return result[1];
}

if (Deno.args.length < 1) {
  console.error("Usage: deno task submit-bulk [solution dir]");
  Deno.exit(1);
}

const solutionsDir = Deno.args[0];
const imagesDir = `${solutionsDir}/images`;
await ensureDirectory(imagesDir);

const apiKey = Deno.env.get("ICFPC_TOKEN");
if (!apiKey) {
  throw new Error("ICFPC_TOKEN is not set");
}

for await (const dirEntry of Deno.readDir(solutionsDir)) {
  if (dirEntry.name.endsWith(".json")) {
    const solutionFile = `${solutionsDir}/${dirEntry.name}`;
    const problemId = extractProblemId(dirEntry.name);
    const json = await Deno.readTextFile(solutionFile);
    const requestBody = {
      problem_id: Number(problemId),
      contents: json,
    };

    console.log(`Submitting ${solutionFile} ...`);
    const response = await fetch("https://api.icfpcontest.com/submission", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        Authorization: `Bearer ${apiKey}`,
      },
      body: JSON.stringify(requestBody),
    });
    if (response.status >= 300) {
      console.error(
        `Failed to submit ${solutionFile}: ${response.status} ${response.statusText}`,
      );
    }
  }
}
