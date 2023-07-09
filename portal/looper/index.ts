import { Context } from "aws-lambda";
import { SolverRunnerEvent } from "../solver_runner";

interface LooperEvent {
    label: string;
    count: number;
    lightningSolver: string;
    pillarSolver: string;
    limit?: number;
    env?: Record<string, string>;
}

type LooperResponseToMap = {
    items: SolverRunnerEvent[];
} & LooperEvent;

interface LooperResponseToEnd {
    end: true
}

type LooperResponse = LooperResponseToMap | LooperResponseToEnd;

const discordWebhook = process.env['DISCORD_WEBHOOK']!;

async function say(message: string): Promise<void> {
    await fetch(discordWebhook, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            content: message
        })
    });
}

export async function handler(
  event: LooperEvent,
  context: Context,
): Promise<LooperResponse> {
    if (event.limit && event.count > event.limit) {
        await say(`${event.label}が終わったんじゃも！`);
        return {
            end: true
        };
    }

    // Make Kolog work
    await say(`${event.label}のループ${event.count}回目をじっこうします！`);

    const items: SolverRunnerEvent[] = [];
    for (let i = 1; i <= 90; ++i) {
        let solverPath: string;
        if (i <= 55) {
            solverPath = `solver/${event.lightningSolver}`;
        } else {
            solverPath = `solver/${event.pillarSolver}`;
        }

        items.push({
            problemId: i,
            solverPath: solverPath,
            solverName: `${event.label}-${event.count}`,
            seed: '__best__',
            env: event.env
        });
    }

    return {
        ...event,
        count: event.count + 1,
        items
    };
}