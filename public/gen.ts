import { loadProblem } from "../painter/core.ts";

interface ProblemAttributes {
    id: string;
    roomWidth: number;
    roomHeight: number;
    numMusicians: number;
    numAttendees: number;
    note?: string;
}

function extractProblemId(path: string): string {
    const result = /(\d+)\.json/.exec(path);
    if (result == null || result.length < 2) {
        throw new Error(`Cannot extract problem ID from ${path}`);
    }
    return result[1];
}

const attrs: ProblemAttributes[] = [];
for await (const dirEntry of Deno.readDir('problems')) {
    if (dirEntry.name.endsWith('.json')) {
        const problem = await loadProblem(`problems/${dirEntry.name}`);
        const id = extractProblemId(dirEntry.name);
        attrs.push({
            id: id,
            roomWidth: problem.room_width,
            roomHeight: problem.room_height,
            numMusicians: problem.musicians.length,
            numAttendees: problem.attendees.length,
        });
    }
}

attrs.sort((a, b) => parseInt(a.id) - parseInt(b.id));

const f = await Deno.open('public/index.html', { create: true, truncate: true, write: true });
const enc = new TextEncoder();

f.write(enc.encode(`
<html>
<head>
<title>Manarimo Portal</title>
<link rel="stylesheet" href="style.css">
</head>
<body>
<h1 class="rainbow-text">
	<span class="block-line"><span><span style="color:#ff0000;">M</span><span style="color:#ff6e00;">a</span><span style="color:#ffdd00;">n</span><span style="color:#b2ff00;">a</span><span style="color:#48ff00;">r</span><span style="color:#00ff26;">i</span><span style="color:#00ff95;">m</span><span style="color:#00fbff;">o&nbsp;</span></span><span><span style="color:#0091ff;">P</span><span style="color:#0022ff;">o</span><span style="color:#4d00ff;">r</span><span style="color:#b700ff;">t</span><span style="color:#ff00d9;">a</span><span style="color:#ff006a;">l</span></span></span>
</h1>
<table id="main-table">
`));

for (const attr of attrs) {
    f.write(enc.encode(`
    <tr>
        <th>${attr.id}</th>
        <td>${attr.roomWidth} x ${attr.roomHeight}</td>
        <td><ul>
            <li>Musicians: ${attr.numMusicians}</li>
            <li>Attendees: ${attr.numAttendees}</li>
        </ul></td>
        <td><img class="thumbnail" src="problem_images/${attr.id}.svg"></td>
    </tr>
    `));
}

f.write(enc.encode(`
</table>
</body>
</html>
`));

f.close();

let sql = 'INSERT INTO problems (problem_id, room_width, room_height, num_musicians, num_attendees) VALUES ';
const values = [];
for (const attr of attrs) {
    values.push(`(${attr.id}, ${attr.roomWidth}, ${attr.roomHeight}, ${attr.numMusicians}, ${attr.numAttendees})`);
}
console.log(sql + values.join(','));