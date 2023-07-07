console.log("musicians attendees");
for (let x = 1; x <= 45; x++) {
   const text = await Deno.readTextFile(`problems/${x}.json`);
   const problem = JSON.parse(text);
   console.log(
      `${x}: ${problem["musicians"].length} ${problem["attendees"].length}`
   );
}
