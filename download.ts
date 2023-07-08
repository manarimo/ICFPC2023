const API = "https://api.icfpcontest.com/problem";

async function download(id: number, path: string) {
    console.log(`Downloading problem ${id}...`);
    const response = await fetch(`${API}?problem_id=${id}`, {
        headers: {
            'Authorization': `Bearer ${Deno.env.get('ICFPC_TOKEN')}`
        }
    });
    if (response.status != 200) {
        throw new Error(`Failed to fetch response: problem id=${id}`);
    }

    const body = await response.json();
    if ('Success' in body) {
        await Deno.writeTextFile(`${path}/${id}.json`, body['Success']);
    } else {
        throw new Error(`API returned failure response: ${body['Failure']}`);
    }
}

async function main() {
    for (let i = 56; i <= 90; i++) {
        await download(i, 'problems');
    }
}

await main();