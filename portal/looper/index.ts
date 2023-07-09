import { Context } from "aws-lambda";
import { SolverRunnerEvent } from "../solver_runner";

interface LooperEvent {
    label: string;
    count: number;
}

interface LooperResponse {
    label: string;
    nextCount: number;
    items: SolverRunnerEvent[];
}

const discordWebhook = process.env['DISCORD_WEBHOOK']!;

export async function handler(
  event: LooperEvent,
  context: Context,
): Promise<LooperResponse> {
    await fetch(discordWebhook, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            content: `${event.label}のループ${event.count}回目をじっこうします！`
        })
    });

    const items: SolverRunnerEvent[] = [];
    for (let i = 1; i <= 90; ++i) {
        if (i <= 55) {
            items.push({
                problemId: i,
                solverPath: "solver/charibert",
                solverName: `${event.label}-${event.count}-charibert`,
                seed: '__best__'
            });
        } else {
            items.push({
                problemId: i,
                solverPath: "solver/charibert_pillar",
                solverName: `${event.label}-${event.count}-charibert`,
                seed: '__best__'
            });
        }
    }

    return {
        label: event.label,
        nextCount: event.count + 1,
        items
    };
}