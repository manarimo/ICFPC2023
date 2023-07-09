const discordWebhook = process.env['DISCORD_WEBHOOK']!;

export async function discordSay(message: string): Promise<void> {
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